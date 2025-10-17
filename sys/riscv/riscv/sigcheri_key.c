#include <sys/types.h>
#include <sys/proc.h>
#include <sys/sigcheri_key.h>
#include <machine/riscvreg.h>

void encrypt_and_store_key(uint64_t keyl, uint64_t keyh, uint64_t buffer[4]){
    buffer[0] = enc_low_mkey(keyl, &buffer[0]);
    buffer[1] = enc_high_mkey(keyl, &buffer[1]);
    buffer[2] = enc_low_mkey(keyh, &buffer[2]);
    buffer[3] = enc_high_mkey(keyh, &buffer[3]);
}

void load_and_decrypt_key(uint64_t *keyl, uint64_t *keyh, uint64_t buffer[4]){
    *keyl = dec_low_mkey(buffer[0], &buffer[0]);
    *keyl |= dec_high_mkey(buffer[1], &buffer[1]);
    *keyh = dec_low_mkey(buffer[2], &buffer[2]);
    *keyh |= dec_high_mkey(buffer[3], &buffer[3]);
}

void construct_new_skey(uint64_t *skeyl, uint64_t *skeyh){
    static volatile int dummy_data = 0;
    int cnt;
    unsigned long key;
    unsigned long tweak;
    #define cycle_read_entroy() ({cnt = 10; while(cnt--){ dummy_data++; }; csr_read(cycle);})

    key = cycle_read_entroy();
    tweak = cycle_read_entroy();
    *skeyl = enc_full_mkey(key, tweak);

    key = cycle_read_entroy();
    tweak = cycle_read_entroy();
    *skeyh = enc_full_mkey(key, tweak);
}

void construct_update_skey(uint64_t key_buffer[4]){
    uint64_t skeyl, skeyh;
    construct_new_skey(&skeyl, &skeyh);
    encrypt_and_store_key(skeyl, skeyh, key_buffer);
    csr_write(skeyl, skeyl);
    csr_write(skeyl, skeyl);
    fence_i();
}

void load_update_skey(uint64_t key_buffer[4]){
    uint64_t skeyl, skeyh;
    load_and_decrypt_key(&skeyl, &skeyh, key_buffer);
    csr_write(skeyl, skeyl);
    csr_write(skeyl, skeyl);
    fence_i();
}

void
key_activate_sw(struct thread *td)
{
    
#if __has_feature(sigcapabilities)
    uint64_t* buffer = td->td_proc->skey_secret_buffer;
    load_update_skey(buffer);
#endif
	
}