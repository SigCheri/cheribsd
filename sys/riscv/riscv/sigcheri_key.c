#include <sys/types.h>
#include <sys/proc.h>
#include <sys/sigcheri_key.h>
#include <sys/systm.h>
#include <machine/riscvreg.h>

#if __has_feature(sigcapabilities)

struct key_pair {
    uint64_t keyl;
    uint64_t keyh;
};

static inline void encrypt_and_store_key(struct key_pair key, uint64_t buffer[4]){
    buffer[0] = enc_low_mkey(key.keyl, &buffer[0]);
    buffer[1] = enc_high_mkey(key.keyl, &buffer[1]);
    buffer[2] = enc_low_mkey(key.keyh, &buffer[2]);
    buffer[3] = enc_high_mkey(key.keyh, &buffer[3]);
}

static inline struct key_pair load_and_decrypt_key(uint64_t buffer[4]){
    struct key_pair key;
    key.keyl = dec_low_mkey(buffer[0], &buffer[0]);
    key.keyl |= dec_high_mkey(buffer[1], &buffer[1]);
    key.keyh = dec_low_mkey(buffer[2], &buffer[2]);
    key.keyh |= dec_high_mkey(buffer[3], &buffer[3]);
    return key;
}

static inline struct key_pair construct_new_skey(void){
    volatile int dummy_data = 0;
    int cnt;
    unsigned long key;
    unsigned long tweak;
    struct key_pair keys;
    #define cycle_read_entroy() ({cnt = 10; while(cnt--){ dummy_data++; }; csr_read(cycle);})

    key = cycle_read_entroy();
    tweak = cycle_read_entroy();
    keys.keyl = enc_full_mkey(key, tweak);

    key = cycle_read_entroy();
    tweak = cycle_read_entroy();
    keys.keyh = enc_full_mkey(key, tweak);

    #undef cycle_read_entroy

    return keys;
}

void construct_update_skey(uint64_t key_buffer[4]){
    struct key_pair key;
    key = construct_new_skey();
    encrypt_and_store_key(key, key_buffer);
    csr_write(skeyl, key.keyl);
    csr_write(skeyh, key.keyh);
    fence_i();
}

void reencrypt_skey(uint64_t buffer_from[4], uint64_t buffer_to[4]){
    struct key_pair key;
    key = load_and_decrypt_key(buffer_from);
    encrypt_and_store_key(key, buffer_to);
}

void load_update_skey(uint64_t key_buffer[4]){
    struct key_pair key;
    key = load_and_decrypt_key(key_buffer);
    csr_write(skeyl, key.keyl);
    csr_write(skeyh, key.keyh);
    fence_i();
}
#else /* !__has_feature(sigcapabilities) */

void construct_update_skey(uint64_t key_buffer[4])
{
    bzero(key_buffer, sizeof(uint64_t) * 4);
}

void reencrypt_skey(uint64_t buffer_from[4], uint64_t buffer_to[4])
{
    bcopy(buffer_from, buffer_to, sizeof(uint64_t) * 4);
}

void load_update_skey(uint64_t key_buffer[4])
{
    (void)key_buffer;
}

#endif /* __has_feature(sigcapabilities) */

void
key_activate_sw(struct thread *td)
{
    
#if __has_feature(sigcapabilities)
    uint64_t* buffer = td->td_proc->skey_secret_buffer;
    load_update_skey(buffer);
#else
    (void)td;
#endif
	
}