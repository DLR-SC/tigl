import json
import os
from pathlib import Path
import pypandoc
import tempfile
import shutil
import subprocess
import urllib.request


def open_input_file(input_file, tmp_dir=None):

    if tmp_dir is None:
        tmp_dir = tempfile.mkdtemp()
    else:
        Path(tmp_dir).mkdir(parents=True, exist_ok=True)

    input_dicts = json.load(open(input_file))

    # download cpacs files, if not local
    for d in input_dicts:
        file = d["cpacs_file"]
        # This can probably be done better
        if file.startswith("http"):
            download_path = os.path.join(tmp_dir, os.path.basename(file))
            urllib.request.urlretrieve(file, download_path)
            d["cpacs_file"] = download_path

    return input_dicts


def create_tiglviewer_script(cpacs_dicts, file_name=None, screenshot_dir=None):

    if screenshot_dir is None:
        screenshot_dir = tempfile.mkdtemp()
    else:
        Path(screenshot_dir).mkdir(parents=True, exist_ok=True)

    content = '''
app.scene.deleteAllObjects();
app.scene.gridOff();

'''

    for d in cpacs_dicts:
        cpacs_file = d["cpacs_file"]
        basename = os.path.basename(cpacs_file)
        content = content + "\napp.scene.deleteAllObjects();\n"
        content = content + "app.openFile(\"" + cpacs_file + "\");\n"

        for v in ["Front", "Back", "Top", "Bottom", "Left", "Right", "Axo"]:
            content = content + "app.viewer.view" + v + "();\napp.viewer.fitAll();"
            content = content + "app.viewer.makeScreenshot(\"" \
                              + os.path.join(screenshot_dir, basename + "-" + v + ".jpg") \
                              + "\");\n"

        if 'extra_screenshots' in d.keys():
            for extra in d["extra_screenshots"]:
                content = content + extra["script"]
                content = content + "app.viewer.makeScreenshot(\"" \
                          + os.path.join(screenshot_dir, basename + "-" + extra["name"] + ".jpg") \
                          + "\");\n"

    if file_name is None:
        f = tempfile.TemporaryFile()
        file_name = f.name
    with open(file_name, 'w') as f:
        f.write(content)

    return file_name, screenshot_dir


def create_screenshots(tigl_script):
    subprocess.run(["tiglviewer-3", "--script", tigl_script])


def generate_pdf_report(cpacs_dicts, output_file, screenshots_expected_dir, screenshot_dir):
    # generate pdf report with markdown and pandoc
    md_string = '''
---
geometry: margin=0.5cm
---

'''

    for d in cpacs_dicts:
        cpacs_file = d["cpacs_file"]
        basename = os.path.basename(cpacs_file)
        md_string = md_string + '\n#' + basename + '\n'
        md_string = md_string + '''
| | Expected | Actual |
| -- | -- | -- |
'''

        for x in ['front', 'back', 'top', 'bottom', 'left', 'right', 'axo']:
            md_string = md_string + '| ' + x + ' | ![](' \
                                  + os.path.join(screenshots_expected_dir, basename + '-' + x + '.jpg') \
                                  + '){width=40%} | ![](' \
                                  + os.path.join(screenshot_dir, basename + '-' + x + '.jpg') \
                                  + '){width=40%} |\n'

        md_string = md_string + "\\newpage\n"

    return pypandoc.convert_text(md_string, 'pdf', format='md', outputfile=output_file)


if __name__ == '__main__':
    input_file = 'cpacs_files.json'
    output_file = 'report.pdf'

    tigl_script = 'create_screenshots.js'
    screenshot_dir = 'screenshots'

    # open input file and download nonlocal files, if any
    cpacs_dicts = open_input_file(input_file, tmp_dir='tmp')

    create_tiglviewer_script(cpacs_dicts,
                             file_name=tigl_script,
                             screenshot_dir=screenshot_dir)

    create_screenshots(tigl_script)

    generate_pdf_report(cpacs_dicts, output_file, screenshots_expected_dir, screenshot_dir)
