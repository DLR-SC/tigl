import argparse
from lxml.etree import XMLParser, parse

XSD_NS = "http://www.w3.org/2001/XMLSchema"


def load_root(schema_file):
    parser = XMLParser(strip_cdata=False)
    tree = parse(schema_file, parser=parser)
    return tree.getroot()


def get_defined_types(root):
    """
    Sammle alle top-level simpleType und complexType Definitionen.
    """
    defined = []

    for elem in root:
        if not isinstance(elem.tag, str):
            continue

        local_name = elem.tag.split("}")[-1]
        if local_name in {"simpleType", "complexType"} and elem.get("name"):
            defined.append(elem.get("name"))

    return defined


def get_used_types(root):
    """
    Sammle alle verwendeten Typnamen aus:
    - type
    - base
    - memberTypes
    Namespace-Präfixe wie 'xsd:string' werden auf 'string' reduziert.
    """
    used = set()

    for elem in root.iter():
        if not isinstance(elem.tag, str):
            continue

        if "type" in elem.attrib:
            used.add(elem.attrib["type"].split(":")[-1])

        if "base" in elem.attrib:
            used.add(elem.attrib["base"].split(":")[-1])

        if "memberTypes" in elem.attrib:
            for member_type in elem.attrib["memberTypes"].split():
                used.add(member_type.split(":")[-1])

    return used


def find_unused_types(root):
    defined_types = get_defined_types(root)
    used_types = get_used_types(root)
    return sorted(t for t in defined_types if t not in used_types)


def main():
    parser = argparse.ArgumentParser(
        description="Listet XSD-Typen auf, die im Schema nicht verwendet werden."
    )
    parser.add_argument(
        "schema_input",
        nargs="?",
        default="cpacs_gen_input/cpacs_schema.xsd",
        help="Pfad zur XSD-Datei (default: cpacs_gen_input/cpacs_schema.xsd)",
    )
    args = parser.parse_args()

    root = load_root(args.schema_input)
    unused_types = find_unused_types(root)

    if not unused_types:
        print("Keine unbenutzten Typen gefunden.")
        return

    print("Unbenutzte Typen:")
    for type_name in unused_types:
        print(type_name)


if __name__ == "__main__":
    main()