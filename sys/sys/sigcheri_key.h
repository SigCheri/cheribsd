#ifndef __KEY_H_
#define	__KEY_H_

#include <sys/types.h>
#include <sys/proc.h>

// void encrypt_and_store_key(uint64_t keyl, uint64_t keyh, uint64_t buffer[4]);

// void load_and_decrypt_key(uint64_t *keyl, uint64_t *keyh, uint64_t buffer[4]);

// void construct_new_skey(uint64_t *skeyl, uint64_t *skeyh);

void construct_update_skey(uint64_t key_buffer[4]);

void load_update_skey(uint64_t key_buffer[4]);

void reencrypt_skey(uint64_t buffer_from[4], uint64_t buffer_to[4]);

void key_activate_sw(struct thread *td);


#endif /* __KEY_H_ */