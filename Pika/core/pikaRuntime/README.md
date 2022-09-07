# pikaRuntime

Here lies code exclussive to the runtime. The gameplay doesn't have acces to it.

The runtime will launch the editor or the game and will provide it all the necessary things (like memory management).

The runtime is versatile and can be launched with different parameters to provide both for the editor or the game.
The shipping version of the game will lik the runtime with the game logic and disable editor specific things at compile time.


--- The process of loading a container

- The runtime will request the dll all the container names
- The runtime will allocate memory for the desired container and ask the dll for memory initialization
- The runtime will call the create function
- The runtime will call the update function
- The runtime will call the deinitializate function
- The runtime will ask the dll to call the destructors for the memory.
- Checks will be done on both realms even if redundant.