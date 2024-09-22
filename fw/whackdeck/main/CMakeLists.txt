idf_component_register(SRCS "main.c" "wavreader.c"
                    INCLUDE_DIRS "."
#                    EMBED_FILES "sound/drama.pcm"
                    PRIV_REQUIRES esp_timer
                    REQUIRES fatfs vfs
                    REQUIRES "tdeck-lib")


