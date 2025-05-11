# .ads format description:
#  first line is 'p ads n m d' where:
#      n is the number of remaining nodes
#      m is the number of remaining edges
#      d is the number of nodes already known to be in the optimal dominating set
#      (those nodes are guaranteed to not be present in the graph)
#  second line is v_1, v_2, ..., v_d, being the list of nodes already known to be in the optimal
#  dominating set
#  following it are n lines describing the nodes of the remaining graph in format 'v s_d s_m e', where:
#      v is the node number in the original graph, note nodes may not be numbered from 1 to n.
#      s_d is the nodes domination status, 0 means undominated, 1 means dominated
#      s_m is the nodes solution membership status, 0 means maybe, 1 means no, 2 means yes.
#      e describes whether the node is a node non-existent in the original graph added by reductions
#          0 means original node,
#          1 means extra node.
# Last m lines describe the edges of the graph in format 'u v f' where:
#      u and v are the nodes being connected
#      f is the edge status, 0 means unconstrained, 1 means forced.
# Comments starting with c can be only at the beginning of the file.

import random
import sys
import xml.etree.ElementTree as ET
from collections import deque

def parse_ads(filename):
    with open(filename) as f:
        lines = [line.strip() for line in f if not line.startswith('c')]
    header = lines[0].split()
    n, m, d = map(int, header[2:5])
    forced_nodes = list(map(int, lines[1].split())) if d > 0 else []
    node_lines = lines[2:2+n]
    nodes = {}
    for line in node_lines:
        v, status_d, status_m, extra = map(int, line.split())
        nodes[v] = {'status': status_d, 'membership': status_m, 'extra': extra, 'neighbors': set()}
    edge_lines = lines[2+n:]
    for line in edge_lines:
        u, v, forced = map(int, line.split())
        nodes[u]['neighbors'].add((v, forced))
        nodes[v]['neighbors'].add((u, forced))
    return nodes

def bfs_subgraph(nodes, start, max_dist):
    visited = set()
    queue = deque()
    distance = {start: 0}
    queue.append(start)
    visited.add(start)
    while queue:
        u = queue.popleft()
        if distance[u] >= max_dist:
            continue
        for v, _ in nodes[u]['neighbors']:
            if v not in visited:
                visited.add(v)
                distance[v] = distance[u] + 1
                queue.append(v)
    return visited

def create_gexf(nodes, subgraph_nodes, start_node, filename):
    gexf = ET.Element('gexf', {
        'xmlns': "http://gexf.net/1.3",
        'xmlns:viz': "http://gexf.net/1.3/viz",
        'xmlns:xsi': "http://www.w3.org/2001/XMLSchema-instance",
        'xsi:schemaLocation': "http://gexf.net/1.3 http://gexf.net/1.3/gexf.xsd",
        'version': "1.3"
    })
    graph = ET.SubElement(gexf, 'graph', defaultedgetype="undirected")
    nodes_el = ET.SubElement(graph, 'nodes')
    edges_el = ET.SubElement(graph, 'edges')

    for v in subgraph_nodes:
        node = ET.SubElement(nodes_el, 'node', id=str(v), label=str(v))
        if nodes[v]['membership'] == 1:
            color = (255, 105, 180)  # różowy
        elif nodes[v]['status'] == 1:
            color = (255, 255, 255)  # biały
        else:
            color = (0, 0, 0)        # czarny
        ET.SubElement(node, 'viz:color', r=str(color[0]), g=str(color[1]), b=str(color[2]), a="1.0")
        size_value = "2.0" if v == start_node else "1.0"
        ET.SubElement(node, 'viz:size', value=size_value)


    edge_id = 0
    created_outside_nodes = set()

    for u in subgraph_nodes:
        has_outside = False
        for v, forced in nodes[u]['neighbors']:
            if v in subgraph_nodes:
                if u < v:
                    edge = ET.SubElement(edges_el, 'edge', id=str(edge_id), source=str(u), target=str(v))
                    ET.SubElement(edge, 'viz:thickness', value="0.1")
                    if forced == 1:
                        ET.SubElement(edge, 'viz:color', r="255", g="0", b="0", a="1.0")
                    edge_id += 1
            else:
                has_outside = True

        if has_outside and u not in created_outside_nodes:
            b = p = 0
            w_labels = []
            b = p = 0
            for v, _ in nodes[u]['neighbors']:
                if v not in subgraph_nodes:
                    status = nodes[v]['status']
                    membership = nodes[v]['membership']
                    if membership == 1:
                        p += 1
                    elif status == 1:
                        w_labels.append(v)
                    elif status == 0:
                        b += 1

            if len(w_labels) == 1:
                label = f"w{w_labels[0]}, b({b}), p({p})"
            else:
                label = f"w({len(w_labels)}), b({b}), p({p})"

            outside_id = f"{label}_for_{u}"
            node = ET.SubElement(nodes_el, 'node', id=outside_id, label=label)
            ET.SubElement(node, 'viz:color', r="0", g="255", b="0", a="1.0")
            ET.SubElement(node, 'viz:size', value="0.5")
            edge = ET.SubElement(edges_el, 'edge', id=str(edge_id), source=str(u), target=outside_id)
            ET.SubElement(edge, 'viz:thickness', value="0.1")
            edge_id += 1
            created_outside_nodes.add(u)

    tree = ET.ElementTree(gexf)
    tree.write(filename, encoding='utf-8', xml_declaration=True)

def main():
    if len(sys.argv) < 3:
        print("Usage: python script.py input.ads output.gexf [max_distance]")
        sys.exit(1)

    random.seed()
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    max_distance = int(sys.argv[3]) if len(sys.argv) > 3 else 5

    nodes = parse_ads(input_file)
    start = random.choice(list(nodes.keys()))
    subgraph_nodes = bfs_subgraph(nodes, start, max_distance)
    create_gexf(nodes, subgraph_nodes, start, output_file)

if __name__ == "__main__":
    main()
