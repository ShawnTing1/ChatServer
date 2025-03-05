# 定义了一个变量SRC_LIST，它包含了当前目录下的所有源文件
aux_source_directory(. SRC_LIST)

# 指定生成可执行文件的名称
add_executable(ChatClient ${SRC_LIST})

# 指定生成可执行文件所需要的链接库
target_link_libraries(ChatClient pthread)