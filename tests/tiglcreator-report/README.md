# TiGL creator tests

This directory contains a list of cpacs files, a 
python script to automatically generate screenshots from the 
cpacs files and a directory containing the expected screenshots. It creates a pdf report that can be used to compare
the generated screenshots to the expected screenshots.

## Requirements

You need a python installation with [pypandoc](https://pypi.org/project/pypandoc/) 
and [click](https://click.palletsprojects.com/en/7.x/). 

```
pip install pypandoc click
```

Pypandoc is a wrapper around pandoc. The Windows and MacOS wheels contain 
pandoc, for Ubuntu, a pandoc installation must be available as well.

Finally, the executable `tiglcreator-3` must be in the path.

## Usage

Run the command
```buildoutcfg
python generate_report.py cpacs_files.json more_cpacs_files.json -o my-report.pdf
```

Enter

```buildoutcfg
python generate_report.py --help
```

for a complete list of options.

## Adding new test cases

Add a test case to [cpacs_files.json](cpacs_files.json).

This json file contains the CPACS files to be used for the report. The file is expected
to be a list of dicts, one per CPACS file.

Each dict must have at least a `"cpacs_file"` key pointing
to the url of the file. The file can be a local path or an online resource. If the path
starts with `"http"` it will be downloaded from the given url.

If the dict has the optional key `"before_script"`, the associated value will be
interpreted as js commands that are to be run before a screenshot is taken.

If the dict has the optional key `"extra_screenshots"`, the associated value will be
interpreted as a list of dicts with the required keys `"name"` and `"script"`. These
dicts can be used to add custom screenshots labeled by `"name"` for the CPACS file. The
script must contain js commands intrepretable by the TiGLCreator js scripting API, that will
be performed before taking the extra screenshot.

Example contents for the input file:

```json
[
    { 
      "cpacs_file":  "input1.xml",
      "before_script": "app.scene.selectAll();\napp.scene.setTransparency(90);\n" 
    },
    {
      "cpacs_file": "input2.xml",
      "extra_screenshots" :
         [
            { "name": "structure",
              "script": "app.scene.selectAll();\napp.scene.setTransparency(90);\n"
            }
        ]
    },
    { "cpacs_file": "https://raw.githubusercontent.com/DLR-SC/tigl-examples/master/cpacs/concorde.cpacs3.xml" }
]
```

Remember to update the expected screenshots when you add new test cases!

## Updating the expected screenshots

Run the command to generate screenshots with the `--update` option:

```
python generate_report.py cpacs_files.json -o my-report.pdf --update
```
