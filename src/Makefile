
COMPILER = gcc

SRC_1 = lab1.c
SRC_2 = lab2.c
SRC_3_ARP = lab3-arp.c
SRC_3_ICMP = lab3-icmp.c
SRC_4 = lab4.c

LIB = 

FLAGS = -Wall -pipe
FLAGS_DEBUG = -g -DDEBUG

BUILD_DIR = build
OUT_1 = lab1
OUT_2 = lab2
OUT_3_ARP = lab3-arp
OUT_3_ICMP = lab3-icmp
OUT_4 = lab4


debug: clean

	mkdir -p $(BUILD_DIR)
	
	$(COMPILER) $(FLAGS) $(FLAGS_DEBUG) $(SRC_1) $(LIB) -o $(BUILD_DIR)/$(OUT_1)
	$(COMPILER) $(FLAGS) $(FLAGS_DEBUG) $(SRC_2) $(LIB) -o $(BUILD_DIR)/$(OUT_2)
	$(COMPILER) $(FLAGS) $(FLAGS_DEBUG) $(SRC_3_ARP) $(LIB) -o $(BUILD_DIR)/$(OUT_3_ARP)
	$(COMPILER) $(FLAGS) $(FLAGS_DEBUG) $(SRC_3_ICMP) $(LIB) -o $(BUILD_DIR)/$(OUT_3_ICMP)
	$(COMPILER) $(FLAGS) $(FLAGS_DEBUG) $(SRC_4) $(LIB) -o $(BUILD_DIR)/$(OUT_4)

clean:

	rm -f $(BUILD_DIR)/*

distclean:

	rm -Rf $(BUILD_DIR)

