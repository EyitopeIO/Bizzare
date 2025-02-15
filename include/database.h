#ifndef DATABASE_H
#define DATABASE_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void db_setup(void);
void db_save(uint64_t timestamp, uint64_t duration_ms, float rx_mb, float tx_mb);


#ifdef __cplusplus
}
#endif

#endif