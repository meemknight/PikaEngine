# dllLoader

-The loading process

The runtime will ask the dll to hand him the container information.
That information will be used by the runtime to corectly set each container memory and flags when loading.
The container itself will be loaded inside the dll realm so the runtime has to corectly know its informations.
