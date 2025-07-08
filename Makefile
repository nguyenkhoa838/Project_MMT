CXX = g++
INCLUDE = -Iincludes
CXXFLAGS = -Wall -std=c++17 $(INCLUDE)
LDFLAGS = -lws2_32 -lgdi32 -luser32
BUILD_DIR = build


SOURCES_DIR = sources
CLIENT_SOURCES = $(SOURCES_DIR)/client.cpp
SERVER_SOURCES = $(SOURCES_DIR)/server.cpp $(SOURCES_DIR)/keylogger.cpp $(SOURCES_DIR)/process_utils.cpp $(SOURCES_DIR)/restart.cpp $(SOURCES_DIR)/shutdown.cpp $(SOURCES_DIR)/screen_capture.cpp $(SOURCES_DIR)/webcam_capture.cpp

# Object files
CLIENT_OBJS = $(patsubst $(SOURCES_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CLIENT_SOURCES))
SERVER_OBJS = $(patsubst $(SOURCES_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SERVER_SOURCES))


all: CXXFLAGS += -O2
all: $(BUILD_DIR)/server.exe $(BUILD_DIR)/client.exe


debug: CXXFLAGS += -g
debug: $(BUILD_DIR)/server.exe $(BUILD_DIR)/client.exe


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(BUILD_DIR)/server.exe: $(SERVER_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)


$(BUILD_DIR)/client.exe: $(CLIENT_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)


clean:
	@echo "Đang xóa các file build..."
	-rm -rf $(BUILD_DIR)

.PHONY: all debug clean
