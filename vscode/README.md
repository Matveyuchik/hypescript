# HypeScript VS Code Extension

Provides syntax highlighting and snippets for the HypeScript language.

## Features
- Keywords: `esli`, `inache`, `poka`, `dlya`, `slomat`, `prodolzhit`, `!HYPE!`
- Built-ins: `pechat`, `vhod`
- Snippets for common constructs

## Install locally

1. Install vsce (once):
```bash
npm install -g @vscode/vsce
```
2. From this folder:
```bash
vsce package
code --install-extension hypescript-language-0.0.1.vsix
```

Alternatively, use VS Code: Run > Extensions: Install from VSIX...

## File Associations
HypeScript files use extensions `.hype` or `.hypescript`.
