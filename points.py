import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from collections import deque
import time


# im not gonna act like i know what this does
# chat gpt wrote all of this ¯\_(ツ)_/¯

TURRET_OFFSET = 5.0  # turret position along X
FADE_TIME = 5.0      # seconds before points disappear

plt.ion()
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

def spherical_to_cartesian(az_deg, el_deg, r):
    az = np.radians(az_deg)
    el = np.radians(el_deg)
    x = r * np.cos(el) * np.cos(az)
    y = r * np.cos(el) * np.sin(az)
    z = r * np.sin(el)
    return x, y, z

def read_new_data(filename, last_offset):
    try:
        with open(filename, "r") as f:
            f.seek(last_offset)
            new_data = f.read()
            new_offset = f.tell()
        entries = [entry.strip() for entry in new_data.split('#') if entry.strip()]
        return entries, new_offset
    except FileNotFoundError:
        return [], last_offset

# Track radar points and turret rays with timestamps
radar_points = deque()
turret_rays = deque()

radar_offset = 0
turret_offset = 0

turret_origin = np.array([TURRET_OFFSET, 0, 0])

while True:
    now = time.time()

    # Read new radar points
    new_radar_entries, radar_offset = read_new_data("radar.txt", radar_offset)
    for entry in new_radar_entries:
        try:
            az, el, dist = map(float, entry.split(','))
            x, y, z = spherical_to_cartesian(az, el, dist)
            radar_points.append((now, (x, y, z)))
        except:
            continue

    # Read new turret rays
    new_turret_entries, turret_offset = read_new_data("turret.txt", turret_offset)
    for entry in new_turret_entries:
        try:
            az, el, dist = map(float, entry.split(','))
            dx, dy, dz = spherical_to_cartesian(az, el, dist)
            tip = turret_origin + np.array([dx, dy, dz])
            turret_rays.append((now, tip))
        except:
            continue

    # Remove expired points
    radar_points = deque([(t, p) for (t, p) in radar_points if now - t <= FADE_TIME])
    turret_rays = deque([(t, p) for (t, p) in turret_rays if now - t <= FADE_TIME])

    # Clear and re-plot
    ax.clear()

    ax.set_xlim(-20, 20)
    ax.set_ylim(-20, 20)
    ax.set_zlim(0, 20)

    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_title('Live 3D Radar and Turret View (fading points)')

    # Plot radar origin and turret origin
    ax.scatter([0], [0], [0], color='red', s=80, label='Radar')
    ax.scatter([turret_origin[0]], [turret_origin[1]], [turret_origin[2]], color='yellow', s=80, label='Turret')

    # Plot radar points
    if radar_points:
        xs, ys, zs = zip(*[p for (_, p) in radar_points])
        ax.scatter(xs, ys, zs, c='green', s=4, label='Radar Points')

    # Plot turret rays
    for (_, tip) in turret_rays:
        xs = [turret_origin[0], tip[0]]
        ys = [turret_origin[1], tip[1]]
        zs = [turret_origin[2], tip[2]]
        ax.plot(xs, ys, zs, color='blue', alpha=0.4, linewidth=0.5)

    ax.legend()
    plt.pause(0.05)
