# C-Multi_Thread-MQTT-Sqlite-Firebase
## Project structure
```
.
├── control_service.sh
├── db_helper.c
├── db_helper.h
├── firebase_helper.c
├── firebase_helper.h
├── install_service.sh
├── main.c
├── Makefile
├── memory_search.c
├── mqtt_client.c
├── mqtt-monitor.service
└── mqtt_thr.h
```

## About Project
โปรเจคนี้แสดงการทำงานของแอพพลิเคชั่น IoT แบบ multi-thread บน Raspberry Pi ที่ใช้คำสั่ง MQTT ในการตรวจสอบหน่วยความจำของระบบ

ส่วนประกอบหลัก:
1.MQTT Thread:
รับข้อมูลจาก EMQX Broker
จัดการคำสั่ง JSON ({"check": "MemTotal"}), ({"check": "MemFree"}), ({"check": "MemAvailable"})
สื่อสารกับการ Search Memory Thread

2.Memory Thread:
อ่านข้อมูลจาก /proc/meminfo
ประมวลผลคำสั่ง (MemTotal, MemAvailable, ฯลฯ)
ส่งข้อมูลไปยัง Database Thread

3.ระบบฐานข้อมูล:
ใช้ SQLite3 เก็บข้อมูลในเครื่อง
บันทึกคำสั่ง, ค่าที่อ่านได้, และเวลา
รับประกันความปลอดภัยของข้อมูล

4.การเชื่อมต่อ Firebase:
อัพโหลดข้อมูลแบบ Real-time
ใช้ REST API ผ่าน libcurl
ติดตามข้อมูลบนคลาวด์

5.Systemd Service:
ทำงานอัตโนมัติเมื่อเปิดเครื่อง
สคริปต์จัดการ service
ทำงานร่วมกับระบบ

## Youtube
https://youtu.be/54EPDwozSCk

## ECE-Flyhigh
Thammasat University TESA 2024
