# Viettel Digital 2023 - Excercise 28.05.2023

## Đề bài
- Dùng module sim `(SIM7020E)` gửi thông tin lên MQTT
- Gửi các dữ liệu `RSRP, RSRQ, SINR, PCI, cellID`
- Chu kì gửi: `5 phút`

## Version Log
- 30.5.2023: `V1.0`   Xây dựng bằng event loop
- 30.5.2023: `V1.0.5` Fix lỗi không lên được mqtt của message "st"
- 31.5.2023: `V1.1`   thêm feature restart kết nối nếu việc truyền gặp vấn đề
  + Gửi lệnh AT không nhận được response
  + Response cho ERROR quá nhiều
  + Bỏ cấu trúc event loop và thay bằng xQueue.

## Dự kiến
- 29.5 -> 31.5: Hoàn thiện các feature
-  1.6 ->  3.6: Phân tích gói bản tin nhận được từ SIM và triển khai bản tin gửi lên cloud (Innoway -  Dự kiến).
-  4.6 ->  6.6: Đẩy bản tin lên cloud, làm dashboard.
-  7.6 ->  9.6: Implement đo thực tế.

## Quá trình làm việc 

