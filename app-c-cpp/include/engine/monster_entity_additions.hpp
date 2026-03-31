// ---- State machine for BasalMonster port ----
enum class MonsterStatus : uint8_t {
    kWaiting   = 4,  // spawn wait phase, not drawn
    kRunning   = 1,
    kAttacking = 2,
    kDying     = 0,  // playing die animation
    kRemove    = 3,  // fully finished, erase from list
};

struct MonsterEntity {
    // --- original fields ---
    int type   = 0;
    bool boss  = false;
    float x    = 0.0f;
    float y    = 0.0f;
    float w    = 0.0f;
    float h    = 0.0f;
    float speed = 0.0f;
    int hp     = 0;
    int max_hp = 0;
    int atk    = 0;
    int atk_cd_ms    = 1000;
    int atk_accum_ms = 0;

    // --- state machine (replaces attacking + slow_until_ms) ---
    MonsterStatus status   = MonsterStatus::kWaiting;
    bool attacking         = false;   // kept for draw-layer compat; set from status
    int  wait_time_ms      = 0;
    int  survive_time_ms   = 0;

    // --- per-type frame layout ---
    int run_frame_num    = 6;
    int atk_frame_num    = 6;
    int atk2_frame_num   = 0;
    int die_frame_num    = 4;
    int jump_frame_num   = 0;
    int atk_judge_frame  = 3;
    int atk_judge_frame2 = 0;
    int remote_missile_frames = 4;
    int missile_blast_frames  = 6;

    // --- attack-range threshold ---
    float monster_atk_x = 0.0f;
    bool  is_remote_atk = false;

    // --- animation state ---
    int   img_id      = 0;
    int   img_time_ms = 0;
    float img_alpha   = 1.0f;
    float die_alpha   = 1.0f;

    // --- status effects ---
    int   shock_time_ms    = 0;
    int   freeze_time_ms   = 0;
    int   burn_time_ms     = 0;
    int   burn_hurt        = 0;
    bool  burn_judge       = false;
    float shock_speed_rate = 1.0f;
    float speed_rate       = 1.0f;
    int   spe_id           = 0;

    // --- be-hit flash / blood bar ---
    int be_hit_cur_ms = 0;
    int blood_show_ms = 0;

    // --- knockback ---
    float blow_dis = 0.0f;

    // --- attack judge ---
    bool atk_judge    = false;

    // --- boss-2 (type 5) ---
    bool  fire_flag            = false;
    bool  summon_flag          = true;
    int   summon_time_ms       = 0;
    int   pending_summon_count = 0;
    int   run_time_ms          = 0;
    float disturbDis           = 0.0f;
    float target_dx            = 1.0f;
    float target_dy            = 0.0f;

    // --- boss-1 (type 4) ---
    int jump_time_ms = 0;

    // --- stone machine (type 6) ---
    int   sto_lag_ms   = 0;
    bool  sto_atk_flag = false;
    float sto_speed_x  = 0.0f;
    float sto_speed_y  = 0.0f;
    float sto_angle    = 0.0f;

    // --- remote missile state (types 3 & 6) ---
    float missile_x       = 0.0f;
    float missile_y       = 0.0f;
    int   remote_img_time = 0;
    int   remote_img_id   = 0;

    // --- river slow ---
    bool  is_in_river    = false;
    float river_slow_rate = 1.0f;
    float delta_drop_y   = 0.0f;

    // --- needs_projectile flag (set by act_* routines, read by update loop) ---
    bool needs_projectile = false;
};