OS         := $(shell uname -s)

TEST_LIB   := gtest

SRC_DIR    := src
TEST_DIR   := src

CXX        := g++
CXXFLAGS   := -Wall -Wextra -Werror -std=c++17

TEST_LDLIB := $(addprefix -l,$(TEST_LIB))

OPEN       := $(if $(filter Linux,$(OS)),xdg-open,open)

GCOV       := --coverage
LCOV       := lcov --no-external -c

VALGRIND   := valgrind --tool=memcheck --trace-children=yes --track-origins=yes --leak-check=full

TEST_SRCS  := $(TEST_DIR)/s21-containers-test.cc
TEST_NAME  := s21-containers-test
LCOV_NAME  := s21-containers.info

REPORT_DIR := report

LINT       := clang-format
LINT_CONF  := .clang-format
LINT_DIR   := materials/linters

CP         := cp -rf

RM         := rm -rf

MAKEFLAGS  += --no-print-directory

all: test

test:
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(TEST_LDLIB) -o $(TEST_NAME)
	./$(TEST_NAME)

.PHONY: test

check-valgrind: test
	CK_FORK=NO $(VALGRIND) ./$(TEST_NAME)

check-style:
	$(CP) $(LINT_DIR)/$(LINT_CONF) $(SRC_DIR)
	find $(SRC_DIR) -name '*.cc' -o -name '*.c' -o -name '*.h' | xargs clang-format -n
	$(RM) $(SRC_DIR)/$(LINT_CONF)

gcov_report:
	$(CXX) $(CXXFLAGS) $(GCOV) $(TEST_SRCS) $(TEST_LDLIB) -o $(TEST_NAME)
	./$(TEST_NAME)
	$(LCOV) -t $(TEST_NAME) -d . -o $(LCOV_NAME)
	genhtml $(LCOV_NAME) -o $(REPORT_DIR)
	$(OPEN) $(REPORT_DIR)/index.html

clean:
	$(RM) $(TEST_NAME)

fclean: clean
	$(RM) $(LCOV_NAME)
	$(RM) $(REPORT_DIR)
	$(RM) *.gcno *.gcda

rebuild:
	$(MAKE) clean
	$(MAKE) all

