import os
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from tqdm import tqdm

base_dir = '.solutions'

dirs = [f"exact_{i:03d}.gr" for i in range(1, 101)]

def plot_histograms(filename, filter_bags, output_filename):
    fig, axes = plt.subplots(10, 10, figsize=(30, 30))
    fig.subplots_adjust(hspace=0.8, wspace=0.8)

    for idx, dir_name in tqdm(list(enumerate(dirs)), desc=f"Processing {filename}"):
        row = idx // 10
        col = idx % 10
        ax = axes[row, col]
        
        histogram_path = os.path.join(base_dir, dir_name, filename)
        if not os.path.isfile(histogram_path):
            ax.text(0.5, 0.5, ":(", fontsize=16, ha='center', va='center', color='red')
            ax.set_xticks([])
            ax.set_yticks([])
            ax.set_title(dir_name, fontsize=8)
            continue

        try:
            with open(histogram_path, 'r') as f:
                lines = f.read().strip().splitlines()
            
            if not lines:
                raise ValueError("Empty file")

            treewidth = int(lines[0])

            if treewidth > 30:
                ax.text(0.5, 0.5, '>30', fontsize=16, ha='center', va='center', color='blue')
                ax.set_xticks([])
                ax.set_yticks([])
            else:
                bag_sizes = []
                counts = []
                for line in lines[1:]:
                    size, count = map(int, line.strip().split())
                    if not filter_bags or size > 13:
                        bag_sizes.append(size)
                        counts.append(count)
                
                if bag_sizes:
                    ax.bar(bag_sizes, counts, color='lightgreen', edgecolor='black')
                    ax.set_xlabel('Bag size', fontsize=6)
                    ax.set_ylabel('Count', fontsize=6)
                    ax.tick_params(axis='both', which='major', labelsize=6)
                    
                    ax.xaxis.set_major_locator(MaxNLocator(integer=True))
                    ax.yaxis.set_major_locator(MaxNLocator(integer=True))
                else:
                    ax.text(0.5, 0.5, 'no data', fontsize=10, ha='center', va='center', color='gray')
                    ax.set_xticks([])
                    ax.set_yticks([])

            ax.set_title(f"{dir_name}\nTreewidth: {treewidth}", fontsize=8)

        except Exception as e:
            ax.text(0.5, 0.5, ":(", fontsize=16, ha='center', va='center', color='red')
            ax.set_xticks([])
            ax.set_yticks([])
            ax.set_title(dir_name, fontsize=8)

    plt.tight_layout()
    plt.savefig(output_filename, dpi=300)
    plt.close()
    print(f"Saved {output_filename}")

tasks = [
    ('histogram.sol', False, 'all_histograms.png'),
    ('histogram.sol', True, 'all_histograms_above13.png'),
    ('histogram_joins.sol', False, 'all_histograms_joins.png'),
    ('histogram_joins.sol', True, 'all_histograms_joins_above13.png'),
]

for filename, filter_bags, output_file in tasks:
    plot_histograms(filename, filter_bags, output_file)
