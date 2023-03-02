file(GET_RUNTIME_DEPENDENCIES 
RESOLVED_DEPENDENCIES_VAR resolved_deps
UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
DIRECTORIES "${search_path}"
POST_INCLUDE_REGEXES "${search_path}"
POST_EXCLUDE_REGEXES ".*"
EXECUTABLES ${target_exe}
)

file(COPY ${resolved_deps} DESTINATION ".")

message(resolved: ${resolved_deps})
message(unresolved: ${unresolved_deps})