#!/usr/bin/env python3
"""Inventory the pinned VC project and expose real Emscripten blockers read-only."""
import argparse
import json
from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET


def resolve_windows(root, relative):
    current = root
    for component in relative.replace('\\', '/').split('/'):
        if component in ('', '.'):
            continue
        if component == '..':
            current = current.parent
            continue
        exact = current / component
        if exact.exists():
            current = exact
            continue
        matches = [p for p in current.iterdir() if p.name.casefold() == component.casefold()] if current.is_dir() else []
        if len(matches) > 1:
            raise ValueError(f'ambiguous Windows path: {relative}')
        current = matches[0] if matches else exact
    return current


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--emxx', required=True)
    parser.add_argument('--project', default='Element/CElementClient/ElementClient.vcproj')
    parser.add_argument('--configuration', default='Release|Win32')
    parser.add_argument('--entry', default='ElementClient.cpp')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = args.source.resolve()
    project = root / args.project
    tree = ET.fromstring(project.read_bytes().decode('gb18030'))
    config = next((c for c in tree.findall('./Configurations/Configuration') if c.attrib['Name'] == args.configuration), None)
    if config is None:
        parser.error('configuration not present in original project')
    tools = {t.attrib['Name']: t.attrib for t in config.findall('Tool')}
    cc = tools['VCCLCompilerTool']
    includes = [resolve_windows(project.parent, p) for p in cc['AdditionalIncludeDirectories'].split(';')]
    sources = []
    for entry in tree.findall('.//File'):
        path = resolve_windows(project.parent, entry.attrib['RelativePath'])
        if path.suffix.lower() not in ('.cpp', '.c', '.cxx'):
            continue
        excluded = any(x.attrib.get('ExcludedFromBuild') == 'true' for x in [x for x in entry.findall('./FileConfiguration') if x.attrib['Name'] == args.configuration])
        sources.append(dict(path=str(path.relative_to(root)), exists=path.is_file(), excluded=excluded))
    command = [args.emxx, '-fsyntax-only', '-ferror-limit=5', '-fms-extensions', '-std=c++17', str(resolve_windows(project.parent, args.entry))]
    for path in includes:
        command += ['-I', str(path)]
    command += ['-D' + d for d in cc['PreprocessorDefinitions'].split(';') if d]
    result = subprocess.run(command, capture_output=True, text=True)
    report = dict(project=args.project, configuration=args.configuration, sources=sources,
                  includes=[dict(path=str(p.relative_to(root)), exists=p.is_dir()) for p in includes],
                  definitions=cc['PreprocessorDefinitions'].split(';'),
                  libraries=tools.get('VCLinkerTool', tools.get('VCLibrarianTool', {})).get('AdditionalDependencies', '').split(),
                  probe=dict(command=command, exitCode=result.returncode, stderr=result.stderr))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(f'{len(sources)} translation units; {sum(not s["exists"] for s in sources)} missing after Windows case resolution')
    print(result.stderr)
    return result.returncode


if __name__ == '__main__':
    raise SystemExit(main())
