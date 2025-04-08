import csv
import os
import re
import sys

# python report.py <directory> generates a csv table from all directory/**/*.report files
# with columns for each algorithm used.
# e.g. 
# file, vc_ds_size, vc_seconds, vc_kbytes, ...
# exact_001.gr, 42, 10.01, 94949, ...
def main(root_dir):
    algorithms = ["defaults", "presolve"]

    headers = ["file"]
    headers += [f"{algo}_ds_size" for algo in algorithms]
    headers += [f"{algo}_seconds" for algo in algorithms]
    headers += [f"{algo}_kbytes" for algo in algorithms]

    rows = []

    for entry in os.listdir(root_dir):
        full_entry_path = os.path.join(root_dir, entry)
        if os.path.isdir(full_entry_path) and re.match(r".*\.gr$", entry):
            row = {"file": entry}
            for algo in algorithms:
                report_path = os.path.join(full_entry_path, f"{algo}.report")
                if os.path.exists(report_path):
                    with open(report_path, "r") as f:
                        data = {}
                        for line in f:
                            parts = line.strip().split()
                            if len(parts) == 2:
                                key, value = parts
                                data[key] = value
                        row[f"{algo}_ds_size"] = data.get("ds_size", "NA")
                        row[f"{algo}_seconds"] = data.get("seconds", "NA")
                        row[f"{algo}_kbytes"] = data.get("kbytes", "NA")
                else:
                    row[f"{algo}_ds_size"] = "NA"
                    row[f"{algo}_seconds"] = "NA"
                    row[f"{algo}_kbytes"] = "NA"
            rows.append(row)

    rows.sort(key=lambda r: r["file"])
    output_csv = "summary.csv"
    with open(output_csv, "w", newline="") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=headers)
        writer.writeheader()
        writer.writerows(rows)

    print(f"Gotowe! Plik zapisany jako {output_csv} w katalogu roboczym.")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Użycie: {sys.argv[0]} <ścieżka_do_katalogu>")
        sys.exit(1)
    main(sys.argv[1])
