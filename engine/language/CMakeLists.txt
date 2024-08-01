add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/expressionLexer.c  ${CMAKE_CURRENT_SOURCE_DIR}/expressionParser.c ${CMAKE_CURRENT_SOURCE_DIR}/expressionLexer.h ${CMAKE_CURRENT_SOURCE_DIR}/expressionParser.h
    COMMAND antlr3 ${CMAKE_CURRENT_SOURCE_DIR}/expression.g > /dev/null 2>&1
    DEPENDS expression.g
)

add_library(language STATIC expressionLexer.c expressionParser.c)
target_link_libraries(language antlr3c)
