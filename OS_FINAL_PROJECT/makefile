# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++17 -Werror -Wsign-conversion -pthread
MEMCHECK_FLAGS = -v --leak-check=full --show-leak-kinds=all --error-exitcode=99 
CACHEGRIND_FLAGS = -v --error-exitcode=99
HELGRIND_FLAGS = -v --error-exitcode=99 
COVERAGE_FLAGS = --coverage

# Source files
graphSrc = $(wildcard Graph/*.cpp)
MSTSrc = $(wildcard MST/*.cpp)
DATASTRUCTSrc = $(wildcard DataStruct/*.cpp) DataStruct/data_structures.hpp
UTILSrc = $(wildcard ServerUtils/*.cpp)


lf-serverSrc = LF-Server.cpp LF/LeaderFollower.cpp
PIPELINE = Pipeline-server.cpp Pipeline/pipelineActiveObject.cpp


# Object files
LF-OBJ = $(graphSrc:.cpp=.o) $(lf-serverSrc:.cpp=.o) $(MSTSrc:.cpp=.o) $(DATASTRUCTSrc:.cpp=.o) $(UTILSrc:.cpp=.o)
PIPELINE-OBJ = $(graphSrc:.cpp=.o) $(PIPELINE:.cpp=.o) $(MSTSrc:.cpp=.o) $(DATASTRUCTSrc:.cpp=.o) $(UTILSrc:.cpp=.o)

#LF-OBJ = $(graphSrc:.cpp=.o) $(lf-serverSrc:.cpp=.o) $(MSTSrc:.cpp=.o) $(UTILSrc:.cpp=.o)
#Pipeline-OBJ = $(graphSrc:.cpp=.o) $(Pipeline:.cpp=.o) $(MSTSrc:.cpp=.o) $(UTILSrc:.cpp=.o)

.PHONY: all  pipeline-server valgrind clean
all: lf-server pipeline-server 

# Valgrind tools: we will check creating 3 graphs and 3 MSTs
# LF - Memory check:
lf-memcheck: lf-server
	@echo "Starting lf-server with Valgrind memcheck..."
	@trap 'kill $$valgrind_pid' SIGINT; \
	valgrind --tool=memcheck $(MEMCHECK_FLAGS) ./lf-server 2>&1 | tee Valgrind-reports/lf-server_memcheck.txt & \
	valgrind_pid=$$!; \
	wait $$valgrind_pid

# LF - CacheGrind:
lf-cachegrind: lf-server
	@echo "Starting lf-server with Valgrind cachegrind..."
	@trap 'kill $$valgrind_pid' SIGINT; \
	valgrind --tool=cachegrind $(CACHEGRIND_FLAGS) ./lf-server 2>&1 | tee Valgrind-reports/lf-server_cachegrind.txt & \
	valgrind_pid=$$!; \
	wait $$valgrind_pid

# LF  - Helgrind:
lf-helgrind: lf-server
	@echo "Starting lf-server with Valgrind helgrind..."
	@trap 'kill $$valgrind_pid' SIGINT; \
	valgrind --tool=helgrind $(HELGRIND_FLAGS) ./lf-server 2>&1 | tee Valgrind-reports/lf-server_helgrind.txt & \
	valgrind_pid=$$!; \
	wait $$valgrind_pid

# Pipeline - Memory check:
pipeline-memcheck: pipeline-server
	@echo "Starting pipeline-server with Valgrind memcheck..."
	@trap 'kill $$valgrind_pid' SIGINT; \
	valgrind --tool=memcheck $(MEMCHECK_FLAGS) ./pipeline-server 2>&1 | tee Valgrind-reports/pipeline-server_memcheck.txt & \
	valgrind_pid=$$!; \
	wait $$valgrind_pid

# Pipeline - Helgrind for pipeline-server:
pipeline-helgrind: pipeline-server
	@echo "Starting pipeline-server with Valgrind helgrind..."
	@trap 'kill $$valgrind_pid' SIGINT; \
	valgrind --tool=helgrind $(HELGRIND_FLAGS) ./pipeline-server 2>&1 | tee Valgrind-reports/pipeline-server_helgrind.txt & \
	valgrind_pid=$$!; \
	wait $$valgrind_pid

# Pipeline - CacheGrind for pipeline-server:
pipeline-cachegrind: pipeline-server
	@echo "Starting pipeline-server with Valgrind cachegrind..."
	@trap 'kill $$valgrind_pid' SIGINT; \
	valgrind --tool=cachegrind $(CACHEGRIND_FLAGS) ./pipeline-server 2>&1 | tee Valgrind-reports/pipeline-server_cachegrind.txt & \
	valgrind_pid=$$!; \
	wait $$valgrind_pid

# LF - Coverage:
compile-lf-coverage: $(LF-OBJ)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) $(LF-OBJ) -o lf-server

lf-lcov_analysis:
	lcov --capture --directory . --output-file Coverage-reports/lf-server_coverage.info
	@echo "Created info file, for furhter analysis run: make lf-html-cov"
	
lf-html-cov:
	genhtml Coverage-reports/lf-server_coverage.info --output-directory Coverage-reports/lf-server
	@echo "Created html report, you can view it by opening Coverage-reports/lf-server/index.html"

# Pipeline - Coverage:
compile-pipeline-coverage: $(PIPELINE-OBJ)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) $(PIPELINE-OBJ) -o pipeline-server

pipeline-lcov_analysis:
	lcov --capture --directory . --output-file Coverage-reports/pipeline-server_coverage.info
	@echo "Created info file, for furhter analysis run: make pipeline-html-cov"
	
pipeline-html-cov:
	genhtml Coverage-reports/pipeline-server_coverage.info --output-directory Coverage-reports/pipeline-server
	@echo "Created html report, you can view it by opening Coverage-reports/pipeline-server/index.html"
# Build targets

html-report: lf-html-cov pipeline-html-cov

lf-server: $(LF-OBJ)
	$(CC) $(CFLAGS) $(LF-OBJ) -o lf-server

pipeline-server: $(PIPELINE-OBJ)
	$(CC) $(CFLAGS) $(PIPELINE-OBJ) -o pipeline-server

# # Compile source files with coverage flags
# %.o: %.cpp
# 	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -c $< -o $@

# Compile source files without coverage flags
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@


# Clean build files
clean:
	rm -f -r *.o Graph/*.o MST/*.o DataStruct/*.o lf-server PIPELINE-server  LF/*.o ServerUtils/*.o PIPELINE/*.o pipeline-server
clean_coverage:
	rm -f -r Coverage-reports/lf-server *.gcno *.gcda *.gcov Graph/*.o Graph/*.gcno Graph/*.gcda Graph/*.gcov MST/*.o MST/*.gcno MST/*.gcda MST/*.gcov DataStruct/*.o DataStruct/*.gcno DataStruct/*.gcda DataStruct/*.gcov ServerUtils/*.o ServerUtils/*.gcno ServerUtils/*.gcda ServerUtils/*.gcov PIPELINE/*.o PIPELINE/*.gcno PIPELINE/*.gcda PIPELINE/*.gcov LF/*.o LF/*.gcno LF/*.gcda LF/*.gcov Coverage-reports/pipeline-server Coverage-reports/lf-server Coverage-reports/pipeline-server Coverage-reports/lf-server
clean_all: clean clean_coverage
	
