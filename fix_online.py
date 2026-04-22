import sqlite3
from datetime import datetime

conn = sqlite3.connect(r'c:\Users\刘睿\yolo-forklift-safety-hub\alarm.db')
c = conn.cursor()
now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

positions = {
    'FORK-001': (630, 760),
    'FORK-002': (630, 760),
    'FORK-003': (630, 760),
}

for device_id, (px, py) in positions.items():
    c.execute(
        "UPDATE devices SET online_status = 1, last_seen = ?, update_time = ?, pos_x = ?, pos_y = ? WHERE device_id = ?",
        (now, now, px, py, device_id)
    )
    print(f"Updated {device_id}: pos=({px}, {py}), online=1")

conn.commit()
conn.close()
print("Done!")
