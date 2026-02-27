import xml.etree.ElementTree as ET
import math
import json

# --- CONFIGURATION ---
INPUT_FILE = 'Nurburgring.gpx'  # Your GPX file (track)
OUTPUT_FILE = 'path.json'       # Always path.json to unify the usage in virtual_sender
TARGET_POINTS = 4096

def haversine_distance(lat1, lon1, lat2, lon2):
    """Calculates distance in meters between two lat/lon points."""
    R = 6371000  # Earth radius in meters
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)
    
    a = math.sin(dphi/2)**2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlambda/2)**2
    return 2 * R * math.atan2(math.sqrt(a), math.sqrt(1 - a))

def resample_track(points, target_count):
    """Resamples a list of (lat, lon) tuples into exactly target_count equidistant points."""
    
    # 1. Calculate cumulative distances
    cumulative_dist = [0.0]
    total_dist = 0.0
    for i in range(1, len(points)):
        dist = haversine_distance(points[i-1][0], points[i-1][1], points[i][0], points[i][1])
        total_dist += dist
        cumulative_dist.append(total_dist)
    
    # 2. Determine step size for exactly target_count points
    step_size = total_dist / (target_count - 1)
    
    new_points = []
    current_dist_target = 0.0
    
    # 3. Interpolate
    # We walk through the original segments and find where the target distance falls
    orig_idx = 0
    
    for i in range(target_count):
        # Handle last point explicitly to avoid rounding errors
        if i == target_count - 1:
            new_points.append(points[-1])
            break
            
        # Find the segment [orig_idx, orig_idx+1] that contains current_dist_target
        while orig_idx < len(cumulative_dist) - 1 and cumulative_dist[orig_idx+1] < current_dist_target:
            orig_idx += 1
            
        # Linear Interpolation
        segment_start_dist = cumulative_dist[orig_idx]
        segment_end_dist = cumulative_dist[orig_idx+1]
        segment_len = segment_end_dist - segment_start_dist
        
        if segment_len == 0:
            fraction = 0
        else:
            fraction = (current_dist_target - segment_start_dist) / segment_len
            
        lat1, lon1 = points[orig_idx]
        lat2, lon2 = points[orig_idx+1]
        
        new_lat = lat1 + (lat2 - lat1) * fraction
        new_lon = lon1 + (lon2 - lon1) * fraction
        
        new_points.append({'lat': round(new_lat, 6), 'lon': round(new_lon, 6)})
        current_dist_target += step_size
        
    return new_points

# --- MAIN EXECUTION ---
print(f"Parsing {INPUT_FILE}...")

# Parse GPX (Handling namespaces)
tree = ET.parse(INPUT_FILE)
root = tree.getroot()
ns = {'gpx': 'http://www.topografix.com/GPX/1/1'} # Standard GPX namespace

# Extract all track points
raw_points = []
for trkpt in root.findall('.//gpx:trkpt', ns):
    lat = float(trkpt.get('lat'))
    lon = float(trkpt.get('lon'))
    raw_points.append((lat, lon))

# If namespace fails, try without (some files are formatted differently)
if not raw_points:
    for trkpt in root.findall('.//trkpt'):
        lat = float(trkpt.get('lat'))
        lon = float(trkpt.get('lon'))
        raw_points.append((lat, lon))

print(f"Found {len(raw_points)} raw points. Resampling to {TARGET_POINTS}...")

# Resample
final_points = resample_track(raw_points, TARGET_POINTS)

# Save to JSON
with open(OUTPUT_FILE, 'w') as f:
    json.dump(final_points, f, indent=None)

print(f"Done! Saved {len(final_points)} coordinates to {OUTPUT_FILE}")