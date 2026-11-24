# Constraints

- **srs.nfr.4.1** The software shall be written in C++20.
- **srs.nfr.4.2** Physics shall use Box2D v3.1.1.
- **srs.nfr.4.3** Rendering shall use raylib 6.2.
- **srs.nfr.4.4** The ECS shall be custom-built by the team.
- **srs.nfr.4.5** The build system shall be CMake.
- **srs.nfr.4.6** The product shall support 2D games only; 3D rendering, 3D physics, multiplayer, gamepad input and mobile, web or console targets are out of scope (Initial Plan, Project Scope).
- **srs.nfr.4.7** Every library, tool and service used shall be free of charge for the team.
- **srs.nfr.4.8** Every library shipped with the product shall have a licence that allows redistribution; the About dialog lists them (srs.fr.34.5).
- **srs.nfr.4.9** The product shall need no hardware beyond a computer that runs one of the operating systems of srs.nfr.3.12.
- **srs.nfr.4.10** The AI language-model service shall be an external Python 3.12 process reached over gRPC with protobuf messages through the AgentBridge service contract (srs.fr.11.10, srs.fr.13.63, srs.fr.33.29).
- **srs.nfr.4.11** The AgentBridge contract shall be kept, owned by Arda Sarıtaş, as version v1 in the code repository and merged before any Editor code calls the AI service (srs.nfr.4.10); it shall define the gRPC health check (srs.fr.13.63), ListModels (srs.fr.33.29), Ask, with the question, context items and model and an answer with citations in the forms of srs.fr.11.4, and the Playtest Agent's Exchange (srs.fr.9.51), with deadlines of 30 s for Ask, 5 s for ListModels and 2 s for each Exchange; UNAVAILABLE and DEADLINE_EXCEEDED shall be reported as srs.nfr.5.7 states, and Stop or closing the AI Assistant panel shall cancel a pending call.
