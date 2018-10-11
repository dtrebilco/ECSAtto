# ECSResearch
Research work into entity component systems


## Overview
[Entity Component System](https://infogalactic.com/info/Entity_component_system)

## Details


#### Objectives
- Targeting games
- No global mutable state
- Speed over safety, but debug checks if you do something bad.

#### Limitations
- Asserts in debug mode when you break an assumption - but in C++ you can still do crazy things like hold pointers.

#### Groups

- Separate out for loading
- Streaming level sections
- have static + dynamic groups
- ability to add to one group while iterating another


#### Component manager registration

- Flag managers
- Custom component

#### Iterators


## Links
- Links to other ECS systems (rust, on github)


---

Do not conform any longer to the pattern of this world, but be transformed by the renewing of your mind.
Then you will be able to test and approve what God's will is - his good, pleasing and perfect will. (Romans 12:2 NIV)

