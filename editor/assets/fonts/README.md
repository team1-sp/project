# Editor fonts

The editor embeds these fonts in its executable (`cmake/EmbedFile.cmake`, DEC-064). They match
the Aegis Editor UI design in Figma, which uses Inter for the interface and JetBrains Mono for
numbers such as the tick counter.

| File | Source | Version | License |
| --- | --- | --- | --- |
| `Inter-Regular.ttf`, `Inter-SemiBold.ttf` | [rsms/inter](https://github.com/rsms/inter/releases/tag/v4.1), `Inter-4.1.zip`, `extras/ttf/` | 4.1 | SIL Open Font License 1.1, [Inter-OFL.txt](Inter-OFL.txt) |
| `JetBrainsMono-Regular.ttf` | [JetBrains/JetBrainsMono](https://github.com/JetBrains/JetBrainsMono/releases/tag/v2.304), `JetBrainsMono-2.304.zip`, `fonts/ttf/` | 2.304 | SIL Open Font License 1.1, [JetBrainsMono-OFL.txt](JetBrainsMono-OFL.txt) |

The toolbar icons come from Font Awesome 6 Free (solid), which rlImGui bundles with its own
license.
