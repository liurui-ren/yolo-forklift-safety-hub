import sqlite3
from datetime import datetime
conn = sqlite3.connect(r'alarm.db')
c = conn.cursor()
now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
c.execute("UPDATE devices SET online_status = 1, last_seen = ?, update_time = ? WHERE device_id LIKE 'FORK-%'", (now, now))
conn.commit()
c.execute('SELECT device_id, online_status, last_seen FROM devices')
for row in c.fetchall():
    print(f"{row[0]}: online={row[1]}, last_seen={row[2]}")
conn.close()
