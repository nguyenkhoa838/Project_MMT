# ===============================
# Cấu hình chung
# ===============================
CXX = g++
INCLUDE = -Iinclude
CXXFLAGS = -Wall -std=c++17 $(INCLUDE)
LDFLAGS = -lws2_32
BUILD_DIR = build

SRC = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SRC))

# ===============================
# Build chế độ Release (mặc định)
# ===============================
all: CXXFLAGS += -O2
all: $(BUILD_DIR)/server.exe $(BUILD_DIR)/client.exe

# ===============================
# Build chế độ Debug
# ===============================
debug: CXXFLAGS += -g
debug: $(BUILD_DIR)/server.exe $(BUILD_DIR)/client.exe

# ===============================
# Tạo thư mục build nếu chưa có
# ===============================
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ===============================
# Biên dịch từng file .cpp thành .o
# ===============================
$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ===============================
# Build server
# ===============================
$(BUILD_DIR)/server.exe: $(BUILD_DIR)/server.o $(BUILD_DIR)/socket_utils.o
	$(CXX) $^ -o $@ $(LDFLAGS)

# ===============================
# Build client
# ===============================
$(BUILD_DIR)/client.exe: $(BUILD_DIR)/client.o $(BUILD_DIR)/socket_utils.o
	$(CXX) $^ -o $@ $(LDFLAGS)

# ===============================
# Dọn dẹp
# ===============================
clean:
	@echo "Đang xóa các file build..."
	-rm -rf $(BUILD_DIR)

.PHONY: all debug clean
