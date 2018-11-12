# ECSAtto
This project is an implementation of an [Entity Component System](https://infogalactic.com/info/Entity_component_system) that is geared towards games.

#### Objectives
There are many ways of implementing an ECS, with different trade offs. Here are the high level goals of this ECS:

- Targeting games.
- Speed over safety, but debug checks at runtime to catch bad code.
- No global mutable state.
- Have O(1) speed in testing if an entity has a component. Due to this, each entity can only one of each component type.
- No limit on the number of component types. Only pay for what components are used.
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
As mentioned previously, this ECS is written for speed. The trade off is that you cannot add to a component type while accessing a component of that type.


```c++
Context<MyGroup> context;
GroupID group = context.AddEntityGroup();

// Create a component
EntityID entity = context.AddEntity(group);
auto newItem = context.AddComponent<MyManager>(entity);

// Will assert here as still holding a reference to newItem
//EntityID entity2 = context.AddEntity(group);
//auto newItem2 = context.AddComponent<MyManager>(entity2);

// Can however add to another group OK
GroupID group2 = context.AddEntityGroup();
EntityID entity3 = context.AddEntity(group2);
auto newItem3 = context.AddComponent<MyManager>(entity3);

// However, this is C++, and you can still do dangerous things with pointers.
// Holding a reference or pointer to data inside a component will no longer 
// provide asserts on bad usage if the pointer outlives the component accessor.
int& a = newItem->a;
```

#### Flag managers

If you do not need to store data in a component, but just a boolean flag value, you can register flag managers.

```c++
// Define a flag manager
class TestFlagManager : public FlagManager {};

//... Register code ....

// Use the flag
context.HasFlag<TestFlagManager>(entity);
context.SetFlag<TestFlagManager>(entity, true);
```

#### Iterators

Also provided is a way of iterating components.

There a three main iterator types for iterating over components:
- **Iter< A >** To iterate over each component of the type. Fastest, but cannot access other component siblings.

- **IterID< A >** If a component stores the entity sub-ID and implements GetSubID(), (eg inherits ComponentTypeIDManager) 
  this iterator can be used. Just as fast as Iter<A> and can access siblings. Useful for sparse components.

- **IterEntity< A >** Iterates each entity in the context, stopping at entities that have the component. 
  Can filter on as many components/flags as necessary. (eg IterEntity<A, B, C, D...> will only stop on entities that have all listed components/flags)
  First filter type must be a component and not a flag.
 
Example usage: 
```c++
       for (auto& i : Iter<A>(context))
       { *i = foo; // Access component data (what methods are available depends on the component)

       for (auto& i : IterID<A>(context))
       { *i = foo;       // Access component data like a pointer
         i.GetEntityID() // Access other components with the entity ID

       for (auto& i : IterEntity<A, B>(context))
       { *i = foo;       // Access component A data like a pointer
         i.GetEntityID() // Entity has component A and component/flag B
```
To restrict iteration to an entity group, pass the group ID as a second argument to any of the iterator types.
Example:
```c++
       for (auto& i : IterEntity<A, B>(context, groupID))
       { i.GetEntityID() // Entity will be in the passed group
```

## Examples

Provided with the code is unit tests (using the Google Test framework) and a example runtime example.

This runtime example demonstrates transforms and bounding volumes in the ECS system. It contains 10,000 static entities and a few dynamic parented entities.

Navigate with the mouse and press "1" to toggle culling from the current view. (to test bounding box culling)

![](./Images/RunTest1.png?raw=true)
![](./Images/RunTest2.png?raw=true)

## Links
Here are some other ECS systems that were found when researching. 

C++
- https://github.com/skypjack/entt
- https://github.com/miguelmartin75/anax
- https://github.com/vinova/Artemis-Cpp
- https://github.com/Yelnats321/EntityPlus
- https://github.com/alecthomas/entityx
- https://github.com/SuperV1234/ecst
- https://github.com/OriginalDecode/ECS
- https://github.com/KayDevs/EntityComponentSystem
- https://github.com/tobias-stein/EntityComponentSystem
- https://github.com/NatWeiss/EntityFu

Rust
- https://github.com/slide-rs/specs Docs: https://slide-rs.github.io/specs/


---

Do not conform any longer to the pattern of this world, but be transformed by the renewing of your mind.
Then you will be able to test and approve what God's will is - his good, pleasing and perfect will. (Romans 12:2 NIV)

