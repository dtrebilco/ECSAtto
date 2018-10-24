# ECSAtto
This project is an implementation of an [Entity Component System](https://infogalactic.com/info/Entity_component_system) that is geared towards games.

#### Objectives
There are many ways of implementing an ECS, with different trade offs. Here are the high level goals of this ECS:

- Targeting games.
- Speed over safety, but debug checks at runtime to catch bad code.
- No global mutable state.
- Have O(1) speed in testing if an entity has a component. Due to this, each entity can only one of each component type.
- No externals but STL. Mostly just usage of std::vector<> that can be swapped if necessary.
- Very fast data oriented design in iterating components. Trades off flexibility of creating/deleting of entities and components. This can be compensated for by smart usage of Entity Groups (see below).

#### Entity Groups
This entity component system uses the concept of Entity Groups. An Entity Group is a grouping of entities that are related or have similar properties. 

This is useful in several scenarios:

- **Async loading** By having multiple ECS contexts, a loading context can create and initialize a large number of entities on another thread, then hand ownership of the loaded groups to the main context when done.  
- **Streaming level sections** Each group can represent a different area of a game map to be loaded/unloaded or turned on/off as needed. 
- **Separate static + dynamic groups** Some groups could be loaded static geometry, while other groups could contain short lived entities. (eg. PFX) This sort of split is recommended to avoid large data moves when creating dynamic entities.
- **Destroy multiple entities at once** Instead of destroying each entity individually, destroying the group will destroy multiple entities at once. (Eg. Destroy the PFX/character groups on level resets)
- **Create components while iterating** This implementation asserts if a component of the same type is created while holding a component handle in the same group. One way of avoiding this is to create components in a different group to the currently iterated one. 

This implementation has limits of 65k groups with 65k entities per group.


#### Quick start code
First copy all the files under the "Lib" directory and include in your project.

Next, take the data you want to put in a component into a component manager class. 

There is a helper template type ComponentTypeManager<> to help with this for the default Array-Of-Structs(AOS) implementation. 

Note: See example Bounds.h to how to do custom and SOA layouts.

```c++
#include <ECS.h>
#include <ECSIter.h>

struct MyData
{
  int a;
  float b;
};

class MyManager : public ComponentTypeManager<MyData> {};
```

Create a group definition to hold the component managers. You need to provide a registration with the group via AddManager() and a template overload way of getting the manager from the group.

```c++
class MyGroup : public EntityGroup
{
public:
  MyGroup()
  {
    AddManager(&m_myManager);
  }
  MyManager m_myManager; // Can be a pointer/smart pointer here to hide component details.
};
template<> inline MyManager& GetManager<MyManager>(MyGroup& i_group) { return i_group.m_myManager; }
```

Now a context can be created and the component used.

```c++
// In runtime code, create a context
Context<MyGroup> context;
GroupID group = context.AddEntityGroup();

// Create the component and assign some values
EntityID entity = context.AddEntity(group);
{
  auto newItem = context.AddComponent<MyManager>(entity);
  newItem->a = 1;
  newItem->b = 2.0f;
}

// Create a second item using in-place constructor
{
  EntityID entity2 = context.AddEntity(group);
  MyData setData{ 1, 2.0f };
  context.AddComponent<MyManager>(entity2, setData);
}

// Iterate items in the context
for(auto& i : Iter<MyManager>(context))
{
  i->a = 2; // Access item data 
}

// Test if component exists
if (context.HasComponent<MyManager>(entity))
{
  // Access component via entity
  auto item = context.GetComponent<MyManager>(entity);
  item->a = 7;
}
```


## Details



#### Limitations
- Asserts in debug mode when you break an assumption - but in C++ you can still do crazy things like hold pointers.



#### Component manager registration

- Flag managers
- Custom component

#### Iterators

## Examples

## Links
- Links to other ECS systems (rust, on github)


---

Do not conform any longer to the pattern of this world, but be transformed by the renewing of your mind.
Then you will be able to test and approve what God's will is - his good, pleasing and perfect will. (Romans 12:2 NIV)

