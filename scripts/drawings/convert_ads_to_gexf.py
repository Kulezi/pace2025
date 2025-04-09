import sys
import xml.etree.ElementTree as ET


def parse_ads(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    lines = [line.strip() for line in lines if line.strip() and not line.startswith('c')]

    header = lines[0].split()
    assert header[0] == 'p' and header[1] == 'ads'
    n, m, d = map(int, header[2:5])

    known_nodes = list(map(int, lines[1].split())) if d > 0 else []

    node_lines = lines[2:2+n]
    edge_lines = lines[2+n:]

    nodes = {}
    for line in node_lines:
        v, s, e = map(int, line.split())
        nodes[v] = {'status': s, 'extra': e}

    edges = []
    for line in edge_lines:
        u, v, f = map(int, line.split())
        edges.append((u, v, f))

    return nodes, edges


def create_gexf(nodes, edges, output_filename):
    gexf = ET.Element('gexf', {
        'xmlns': 'http://gexf.net/1.3',
        'xmlns:viz': 'http://gexf.net/1.3/viz',
        'xmlns:xsi': 'http://www.w3.org/2001/XMLSchema-instance',
        'xsi:schemaLocation': 'http://gexf.net/1.3 http://gexf.net/1.3/gexf.xsd',
        'version': '1.3'
    })

    graph = ET.SubElement(gexf, 'graph', {'defaultedgetype': 'undirected'})

    nodes_element = ET.SubElement(graph, 'nodes')
    for node_id, data in nodes.items():
        node_elem = ET.SubElement(nodes_element, 'node', {
            'id': str(node_id),
            'label': str(node_id)
        })

        color = {'r': '255', 'g': '255', 'b': '255'} if data['status'] == 1 else {'r': '0', 'g': '0', 'b': '0'}
        ET.SubElement(node_elem, 'viz:color', {**color, 'a': '1.0'})

    edges_element = ET.SubElement(graph, 'edges')
    for idx, (u, v, forced) in enumerate(edges):
        edge_elem = ET.SubElement(edges_element, 'edge', {
            'id': str(idx),
            'source': str(u),
            'target': str(v)
        })
        if forced:
            ET.SubElement(edge_elem, 'viz:color', {'r': '255', 'g': '0', 'b': '0', 'a': '1.0'})

    tree = ET.ElementTree(gexf)
    tree.write(output_filename, encoding='utf-8', xml_declaration=True)


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.ads output.gexf")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    nodes, edges = parse_ads(input_file)
    create_gexf(nodes, edges, output_file)


if __name__ == "__main__":
    main()
