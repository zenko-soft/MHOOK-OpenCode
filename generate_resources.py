import os
import struct

mhook_dir = r"C:\Programs\mhook"
output_bin = r"C:\Programs\mhook\source mhook\MHookResources.bin"

files = []
for f in os.listdir(mhook_dir):
    if f.upper().endswith('.MHOOK'):
        full_path = os.path.join(mhook_dir, f)
        stat = os.stat(full_path)
        files.append({
            'name': f,
            'path': full_path,
            'time': stat.st_mtime
        })

files.sort(key=lambda x: x['time'], reverse=True)

top_50 = files[:50]
rest = files[50:]
rest.sort(key=lambda x: x['name'].lower())

all_files = top_50 + rest

print(f"Total files: {len(files)}")
print(f"Top 50 by date: {len(top_50)}")
print(f"Rest (alphabetical): {len(rest)}")

with open(output_bin, 'wb') as out:
    for f in all_files:
        name = f['name']
        with open(f['path'], 'rb') as fp:
            data = fp.read()
        
        name_bytes = name.encode('utf-16-le')
        name_len = len(name_bytes)
        
        out.write(struct.pack('<I', name_len))
        out.write(name_bytes)
        out.write(struct.pack('<I', len(data)))
        out.write(data)

print(f"Written to {output_bin}")
