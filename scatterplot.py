import os
import re

import matplotlib.pyplot as plt

# 1. Add all the filenames you want to graph into this list
file_list = ["no_cache.csv", "cache_array.csv"]

# Loop through each file in your list
for file_path in file_list:
    x_coords = []
    y_coords = []

    try:
        with open(file_path, "r") as file:
            for line in file:
                # Find all numbers (integers or decimals, positive or negative)
                numbers = re.findall(r"[-+]?\d*\.\d+|\d+", line)
                if len(numbers) == 2:
                    x_coords.append(float(numbers[0]))
                    y_coords.append(float(numbers[1]))

        # Plot the points for this specific file
        # Matplotlib automatically rotates colors for each separate .scatter() call
        # os.path.basename(file_path) extracts just the file name if a path is provided
        plt.scatter(
            x_coords, y_coords, marker="o", s=20, label=os.path.basename(file_path)
        )

    except FileNotFoundError:
        print(f"Warning: The file '{file_path}' was not found. Skipping it.")

# Add titles and axis labels
plt.title("Multi-File Scatter Plot Comparison")
plt.xlabel("Num creatures")
plt.ylabel("Update time (ms)")
plt.grid(True)

# Add the legend to display the filename labels and their corresponding colors
plt.legend(loc="best")

# Save the final scatter plot as an image file
plt.savefig("combined_scatter_plot.png", bbox_inches="tight")
print("Graph successfully created and saved as 'combined_scatter_plot.png'")
