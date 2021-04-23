import click
import datetime
import json
import os
from pathlib import Path
import pypandoc
import tempfile
import shutil
import subprocess
import urllib.request


def open_input_file(input_file, download_dir):
    """
    Opens a json file containing a description of the CPACS files used to generate
    the report. These can contain remote locations, in this case, the remote files
    will be downloaded to the download_dir.

    A json file containing the CPACS files to be used for the report. The file is expected
    to be a list of dicts, one per CPACS file.

    Each dict must have at least a "cpacs_file" key pointing
    to the url of the file. The file can be a local path or an online resource. If the path
    starts with \"http\" it will be downloaded from the given url.

    If the dict has the optional key \"extra_screenshots\", the associated value will be
    interpreted as a list of dicts with the required keys \"name\" and \"script\". These
    dicts can be used to add custom screenshots labeled by \"name\" for the CPACS file. The
    script must contain js commands intrepretable by the TiGLViewer js scripting API, that will
    be performed before taking the extra screenshot.

    Example contents for the input file:

    [
        {
          "cpacs_file": "input.xml",
          "extra_screenshots" :
             [
                { "name": "structure",
                  "script": "app.scene.selectAll();\napp.scene.setTransparency(90);\n"
                }
            ]
        },
        {
            "cpacs_file": "https://raw.githubusercontent.com/DLR-SC/tigl-examples/master/cpacs/concorde.cpacs3.xml"
        }
    ]

    :param input_file: json file describing the CPACS files.
    :param download_dir: A directory for the location of downloaded
                    cpacs files
    :return: The parsed dictionary, where remote files are replaced by downloaded files
    """

    input_dicts = json.load(open(input_file))

    # download cpacs files, if not local
    for d in input_dicts:
        file = d["cpacs_file"]
        # This can probably be done better
        if file.startswith("http"):
            download_path = os.path.join(download_dir, os.path.basename(file))
            urllib.request.urlretrieve(file, download_path)
            d["cpacs_file"] = download_path

    return input_dicts


def create_tiglviewer_script(cpacs_dicts, filename, screenshot_dir):
    """
    creates a tigl js script that takes the screenshots. It is generated from
    the json input file opened with open_input_file, where all remote paths must
    be replaced by local copies.

    :param cpacs_dicts: The input json file
    :param filename: filename of the script file for writing
    :param screenshot_dir: A directory for storing the screenshots
    :return: None
    """

    content = '''
app.scene.deleteAllObjects();
app.scene.gridOff();

'''

    def add_screenshot_command(content, screenshot_path):
        content = content + "app.viewer.view" + v + "();\napp.viewer.fitAll();"
        content = content + "app.viewer.makeScreenshot(\"" \
                  + screenshot_path \
                  + "\");\n"
        return content

    for d in cpacs_dicts:
        cpacs_file = Path(d["cpacs_file"]).as_posix()
        basename = os.path.basename(cpacs_file)
        content = content + "\napp.scene.deleteAllObjects();\n"
        content = content + "app.openFile(\"" + cpacs_file + "\");\n"

        for v in ["Front", "Back", "Top", "Bottom", "Left", "Right", "Axo"]:
            screenshot_path = Path(os.path.join(screenshot_dir, basename + "-" + v + ".jpg")).as_posix()
            content = add_screenshot_command(content, screenshot_path)

        if 'extra_screenshots' in d.keys():
            for extra in d["extra_screenshots"]:
                screenshot_path = Path(os.path.join(screenshot_dir, basename + "-" + extra["name"] + ".jpg")).as_posix()
                content = content + extra["script"]
                content = add_screenshot_command(content, screenshot_path)

    content = content + "\napp.close();\n"

    with open(filename, 'w') as file:
        file.write(content)


def create_screenshots(tigl_script):
    """
    This calls tiglviewer-3 with the provided tigl script
    :param tigl_script: A js TiGL script
    :return: None
    """
    subprocess.run(["tiglviewer-3", "--script", tigl_script])


def generate_pdf_report(cpacs_dicts, output_file, screenshots_expected_dir, screenshot_dir):
    """
    This file creates the actual pdf report comparing screenshots generated with tiglviewer-3.

    :param cpacs_dicts: A dictionary describing the cpacs files
    :param output_file: An output path for the generated report
    :param screenshots_expected_dir: The directory containing the \"expected\" images
    :param screenshot_dir: The directory containing the generated images
    :return: Forwards the pandoc output
    """
    md_string = '''
---
geometry: margin=0.5cm
header-includes: \\usepackage{color}
---

'''

    with open(os.path.join(screenshots_expected_dir, 'date.txt'), 'r') as f:
        today = f.read()

    for d in sorted(cpacs_dicts, key=lambda k: os.path.basename(k['cpacs_file'])):
        cpacs_file = d["cpacs_file"]
        basename = os.path.basename(cpacs_file)
        md_string = md_string + '\n# ' + basename + '\n'
        md_string = md_string + "| | Expected (" + today + ") | Actual |\n| -- | -- | -- |\n"

        columns = ['Front', 'Back', 'Top', 'Bottom', 'Left', 'Right', 'Axo']
        if 'extra_screenshots' in d.keys():
            for extra in d["extra_screenshots"]:
                columns.append(extra['name'])

        for x in columns:
            expected_screenshot_path = os.path.join(screenshots_expected_dir, basename + '-' + x + '.jpg')
            if os.path.exists(expected_screenshot_path):
                expected_screenshot_string = "![](" + expected_screenshot_path + "){width=40%}"
            else:
                expected_screenshot_string = "**\\textcolor{red}{**FAILED**}**"

            screenshot_path = os.path.join(screenshot_dir, basename + '-' + x + '.jpg')
            if os.path.exists(expected_screenshot_path):
                screenshot_string = "![](" + screenshot_path + "){width=40%}"
            else:
                screenshot_string = "**\\textcolor{red}{**FAILED**}**"

            md_string = md_string + '| ' + x + ' | ' \
                                  + expected_screenshot_string \
                                  + ' | ' \
                                  + screenshot_string \
                                  + ' |\n'

        md_string = md_string + "\\newpage\n"

    return pypandoc.convert_text(md_string, 'pdf', format='md', outputfile=output_file)

@click.command(help='''
Generates a tigl viewer report that compares expected screenshots to newly generated screenshots, given a list of CPACS files
''')
@click.option('-i', '--input-file', required=True, type=click.Path(), help='''
A json file containing the CPACS files to be used for the report. The file is expected
to be a list of dicts, one per CPACS file.
 
Each dict must have at least a "cpacs_file" key pointing 
to the url of the file. The file can be a local path or an online resource. If the path
starts with \"http\" it will be downloaded from the given url.

If the dict has the optional key \"extra_screenshots\", the associated value will be
interpreted as a list of dicts with the required keys \"name\" and \"script\". These
dicts can be used to add custom screenshots labeled by \"name\" for the CPACS file. The
script must contain js commands intrepretable by the TiGLViewer js scripting API, that will 
be performed before taking the extra screenshot.

Example contents for the input file: 

[{"cpacs_file": "input.xml", "extra_screenshots" : [ { "name": "structure", "script": "app.scene.selectAll();\napp.scene.setTransparency(90);\n"}]},{"cpacs_file": "https://raw.githubusercontent.com/DLR-SC/tigl-examples/master/cpacs/concorde.cpacs3.xml"}]


''')
@click.option('-o', '--output-file', required=True, type=click.Path(), help='''
The path for the generated report.
''')
@click.option('--update', is_flag=True, default=False, help='''
With --update, the \"expected\" images will be updated with the newly generated 
images.
''')
@click.option('--tigl-script', default=None, type=click.Path(), help='''
An optional path for the generated tigl script. If not provided, a temporary file will
be created in a system default temporary directory.
''')
@click.option('--screenshot-dir', default=None, type=click.Path(), help='''
An optional path for the saved screenshots. If not provided,  a system default temporary 
directory will be used.
''')
@click.option('--download-dir', default=None, type=click.Path(), help='''
An optional directory that is used as a download location for remote cpacs files. 
If not provided, a system default temporary directory will be used.
''')
def main(input_file, output_file, update, tigl_script, screenshot_dir, download_dir):

    screenshots_expected_dir = 'screenshots-expected'

    # create temporary screenshot dir, if none is provided
    if screenshot_dir is None:
        screenshot_dir = tempfile.mkdtemp()
    else:
        Path(screenshot_dir).mkdir(parents=True, exist_ok=True)

    # create temporary download dir, if none is provided
    if download_dir is None:
        download_dir = tempfile.mkdtemp()
    else:
        Path(download_dir).mkdir(parents=True, exist_ok=True)

    # create temporary tigl_script file, if none is provided
    if tigl_script is None:
        f = tempfile.NamedTemporaryFile(mode='w', delete=False)
        tigl_script = f.name

    # open input file and download nonlocal files, if any
    cpacs_dicts = open_input_file(input_file, download_dir)

    # create the tiglviewer-3 script that generates screenshots
    create_tiglviewer_script(cpacs_dicts,
                             tigl_script,
                             screenshot_dir)

    # take the screenshots using tiglviewer-3
    create_screenshots(tigl_script)

    if update:
        # copy the new screenshots to the expected folder and update the
        # date
        if os.path.exists(screenshots_expected_dir) and os.path.isdir(screenshots_expected_dir):
            shutil.rmtree(screenshots_expected_dir)
        shutil.copytree(screenshot_dir, screenshots_expected_dir)
        with open(os.path.join(screenshots_expected_dir, 'date.txt'), 'w') as f:
            f.write(str(datetime.datetime.now().date()))

    # generate the screenshot report using pandoc
    generate_pdf_report(cpacs_dicts,
                        output_file,
                        screenshots_expected_dir,
                        screenshot_dir)


if __name__ == '__main__':
    main()
