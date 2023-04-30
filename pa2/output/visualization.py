import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

data= []
# Read the data from the input file
with open('output.rpt', 'r') as f:
    for _ in range(5):
        next(f)
    data = [line.strip().split() for line in f]

print(data)

# Convert the data to the appropriate data types
rect_names = [row[0] for row in data]
x1_vals = [float(row[1]) for row in data]
y1_vals = [float(row[2]) for row in data]
x2_vals = [float(row[3]) for row in data]
y2_vals = [float(row[4]) for row in data]

print(x1_vals)
# Create a new figure
fig, ax = plt.subplots()

# Add rectangles to the plot
for i in range(len(data)):
    rect = plt.Rectangle((x1_vals[i], y1_vals[i]), x2_vals[i] - x1_vals[i], y2_vals[i] - y1_vals[i], alpha=0.5, edgecolor='black')
    x = int(x1_vals[i] + (x2_vals[i] - x1_vals[i]) / 2)
    y = int(y1_vals[i] + (y2_vals[i] - y1_vals[i]) / 2)
    ax.add_patch(rect)
    ax.annotate(rect_names[i], xy=(x1_vals[i], y1_vals[i]), xytext=(5, 5), textcoords='offset points', color='blue', fontsize=10, fontweight='bold')
    ax.annotate('({},{})'.format(x, y), xy=(x, y), xytext=(5, 5), textcoords='offset points', color='red', fontsize=10, fontweight='bold')
    ax.scatter(x, y, color='red', s=10)


# Set the axis limits
ax.set_xlim(min(x1_vals), max(x2_vals) + 10)
ax.set_ylim(min(y1_vals), max(y2_vals) + 10)

# Set the axis labels
ax.set_xlabel('X-axis')
ax.set_ylabel('Y-axis')

# Set the title
ax.set_title('Rectangles')

# Show the plot
plt.show()
