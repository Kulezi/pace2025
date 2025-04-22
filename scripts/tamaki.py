import os
import csv

def collect_tw_tamaki(working_dir):
    results = []

    for dirpath, _, filenames in os.walk(working_dir):
        if 'tw_tamaki.sol' in filenames:
            sol_path = os.path.join(dirpath, 'tw_tamaki.sol')
            folder_name = os.path.basename(dirpath)

            try:
                with open(sol_path, 'r') as f:
                    line = f.readline().strip()
                    if not line or line.upper() == "FAILED":
                        value = "NA"
                    else:
                        value = int(line)
                results.append({'filename': folder_name, 'tw_tamaki': value})
            except Exception as e:
                print(f"Error reading {sol_path}: {e}")
                results.append({'filename': folder_name, 'tw_tamaki': "NA"})

    results.sort(key=lambda x: x['filename'])
    return results


def write_csv(results, output_path):
    with open(output_path, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=['filename', 'tw_tamaki'])
        writer.writeheader()
        for row in results:
            writer.writerow(row)


if __name__ == "__main__":
    WORKINGDIR = "./.solutions"
    OUTPUT_CSV = "tw_tamaki_summary.csv"

    data = collect_tw_tamaki(WORKINGDIR)
    write_csv(data, OUTPUT_CSV)
    print(f"Summary written to {OUTPUT_CSV}")
