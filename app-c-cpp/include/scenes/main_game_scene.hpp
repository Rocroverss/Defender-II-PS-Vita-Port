#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

#include "scenes/scene.hpp"

namespace defender {

class MainGameScene : public Scene {
public:
    using TransitionRequest = std::function<void(int scene_id, int direction)>;

    explicit MainGameScene(TransitionRequest transition_cb);

    void set_replay_mode(bool replay_mode);
    void set_online_mode(bool online_mode);
    bool need_rec() const;
    bool is_rep() const;
    bool is_game_over() const { return is_game_over_; }
    bool is_game_finished() const { return is_game_finish_; }
    bool is_boss_active() const { return boss_spawned_ && !boss_killed_ && !is_game_over_ && !is_game_finish_; }
    void preload_assets();

    void random_init();
    void reset() override;

    bool touch(const TouchEvent& event) override;
    void draw() override;
    void update() override;

    // Public so static helper functions in .cpp files can access these types
    // without requiring friend declarations.
    enum class MonsterStatus : uint8_t {
        kWaiting   = 4,  // spawn wait phase, not drawn
        kRunning   = 1,
        kAttacking = 2,
        kDying     = 0,  // playing die animation
        kRemove    = 3,  // fully finished, erase from list
    };

    struct MonsterEntity {
        // --- original fields ---
        uint64_t uid  = 0;
        int   type  = 0;
        bool  boss  = false;
        float x     = 0.0f;
        float y     = 0.0f;
        float w     = 0.0f;
        float h     = 0.0f;
        float speed = 0.0f;
        int   hp     = 0;
        int   max_hp = 0;
        int   atk    = 0;
        int   atk_cd_ms    = 1000;
        int   atk_accum_ms = 0;

        // --- state machine (replaces attacking + slow_until_ms) ---
        MonsterStatus status       = MonsterStatus::kWaiting;
        bool          attacking    = false;  // kept for draw-layer compat
        int           wait_time_ms = 0;
        int           survive_time_ms = 0;

        // --- per-type frame layout ---
        int run_frame_num         = 6;
        int atk_frame_num         = 6;
        int atk2_frame_num        = 0;
        int die_frame_num         = 4;
        int jump_frame_num        = 0;
        int atk_judge_frame       = 3;
        int atk_judge_frame2      = 0;
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
        int   freeze_time_ms   = 0;  // countdown ms; replaces slow_until_ms
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
        bool atk_judge = false;

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
        bool  is_in_river     = false;
        float river_slow_rate = 1.0f;
        float delta_drop_y    = 0.0f;

        // --- flag set by act_* routines, consumed by update loop ---
        bool needs_projectile = false;
    };

    // ---- Magic system ----
    struct MagicInstance {
        int   magic_type  = 0;
        int   power       = 0;
        int   spe_time    = 0;
        float x           = 0.0f;
        float y           = 0.0f;
        float target_x    = 0.0f;
        float target_y    = 0.0f;
        int   lag_ms      = 0;
        int   elapsed_ms  = 0;
        bool  active      = false;
        bool  finished    = false;
        int   img_id      = 0;
        int   img_time_ms = 0;
        bool  blasting    = false;
        int   blast_ms    = 0;
        bool  damage_applied = false;
    };

private:
    struct ArrowEntity {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        int power = 0;
    };

    struct EffectEntity {
        float x      = 0.0f;
        float y      = 0.0f;
        float radius = 0.0f;
        float ttl_ms = 0.0f;
        float r      = 1.0f;
        float g      = 1.0f;
        float b      = 1.0f;
        int   kind   = 0;
        float w      = 0.0f;
        float h      = 0.0f;
        float vx     = 0.0f;
        float vy     = 0.0f;
    };

    struct EnemyProjectile {
        int   kind        = 0;
        float x           = 0.0f;
        float y           = 0.0f;
        float vx          = 0.0f;
        float vy          = 0.0f;
        int   damage      = 0;
        bool  blasting    = false;
        float lifetime_ms = 0.0f;
    };

    struct SpellSpec {
        int   mana_cost = 0;
        int   damage    = 0;
        float radius    = 0.0f;
    };

    struct TurretMissile {
        bool     active = false;
        bool     blasting = false;
        float    x = 0.0f;
        float    y = 0.0f;
        float    vx = 0.0f;
        float    vy = 0.0f;
        float    accel_x = 0.0f;
        float    delta_vy = 0.0f;
        int      blast_time_ms = 0;
        uint64_t target_uid = 0;
    };

    struct TurretState {
        bool  active = false;
        float x = 0.0f;
        float y = 0.0f;
        float bob_y = 0.0f;
        float alpha = 1.0f;
        int   bg_frame = 0;
        int   attack = 0;
        int   attack_range_px = 0;
        int   cooldown_ms = 1500;
        int   attack_time_ms = 0;
        int   blast_range_px = 0;
        int   blast_damage = 0;
        uint64_t target_uid = 0;
        std::array<TurretMissile, 5> missiles{};
    };

    struct MagicSlot {
        int magic_type   = 0;
        int mana_cost    = 0;
        int power        = 0;
        int spe_time     = 0;
        int range_units  = 0;
        int total_cd_ms  = 10000;
        int cd_left_ms   = 10000;
    };

    void configure_stage();
    void update_wave();
    int random_stage_monster_type();
    int add_formation(int form_type, int number, int mons_type, int check_time);
    int row_form(int number, int mons_type, int check_time);
    int one_form(int number, int mons_type, int check_time);
    int goose_form(int number, int mons_type, int check_time);
    void spawn_stage_bosses();
    void spawn_monster(bool boss, int forced_type, int wait_ms = 0, int mons_y_units = -10);
    void update_monsters(float dt_ms);
    void update_arrows(float dt_ms);
    void update_enemy_projectiles(float dt_ms);
    void update_wall_state(float dt_ms);
    void init_wall_defenders();
    void update_wall_defenders(float dt_ms);
    void update_mission_state();
    void update_effects(float dt_ms);
    void shoot_volley(float target_x, float target_y);
    void cast_spell_fire(float x, float y);
    void cast_spell_ice(float x, float y);
    void cast_spell_lightning(float x, float y);
    void apply_aoe_damage(float x, float y, float radius, int damage, int apply_slow_ms);
    void kill_monster(size_t idx);
    void spawn_enemy_projectile(const MonsterEntity& mon);
    bool panel_touch(const TouchEvent& event);
    void draw_wall_defender_layer();
    void draw_auto_turrets();
    void draw_wall_layer(float hp_ratio);
    void draw_arrow_layer();
    void draw_bow_layer();
    void draw_effect_layer();
    void draw_panel_layer(float hp_ratio, float mana_ratio);
    void warmup_textures();

    // Magic subsystem
    void magic_show_range(float x, float y, int range_units);
    void magic_update_range(float x, float y);
    void magic_cancel();
    void magic_cast(int magic_type, int power, int spe_time, float x, float y);
    void magic_add(int magic_type, int power, int spe_time,
                   float x, float y, int lag_ms);
    void magic_update(float dt_ms);
    void magic_update_java(float dt_ms);
    void magic_draw();
    void magic_apply_hit(const MagicInstance& inst);
    bool monster_be_hit(size_t idx, int hit_point, int hit_type);
    void monster_apply_magic_effect(size_t idx, int magic_type, int power, int spe_time);
    void init_bow_state();
    void init_magic_state();

    TransitionRequest transition_cb_;

    std::mt19937 form_rng_;
    std::mt19937 boss1_rng_;
    std::mt19937 boss2_rng_;

    std::vector<ArrowEntity>     arrows_;
    std::vector<MonsterEntity>   monsters_;
    std::vector<EnemyProjectile> enemy_projectiles_;
    std::vector<EffectEntity>    effects_;
    std::array<TurretState, 2>   wall_defenders_{};

    bool  magic_show_range_  = false;
    float magic_target_x_    = 0.0f;
    float magic_target_y_    = 0.0f;
    int   magic_range_px_    = 0;
    std::vector<MagicInstance> magic_using_;
    std::vector<MagicInstance> magic_recycle_;
    std::array<MagicSlot, 3> magic_slots_{};

    std::array<int, 8> section_monster_num_{};
    std::array<int, 8> section_time_sec_{};
    std::array<int, 8> section_spawned_{};

    float bow_x_            = 0.0f;
    float bow_y_            = 0.0f;
    float bow_angle_deg_    = 0.0f;
    float target_angle_deg_ = 0.0f;

    float wall_x_              = 0.0f;
    int   wall_max_hp_         = 1000;
    int   wall_hp_             = 1000;
    int   wall_max_mana_       = 100;
    int   wall_mana_           = 50;
    float mana_regen_per_sec_  = 7.0f;
    float mana_accum_          = 0.0f;
    int   add_mana_bonus_      = 20;
    int   add_mana_cd_ms_      = 5000;
    int   add_mana_cd_left_ms_ = 0;

    int   arrow_power_    = 42;
    int   arrow_num_      = 1;
    float arrow_speed_    = 980.0f;
    int   bow_cd_ms_      = 260;
    int   bow_cd_left_ms_ = 0;

    int  active_section_       = 1;
    int  stage_total_time_sec_ = 55;
    int  spawn_frequency_ms_   = 3000;
    int  spawn_accum_ms_       = 0;
    int  total_spawn_target_   = 0;
    int  total_spawned_        = 0;
    bool boss_spawned_         = false;
    bool boss_killed_          = false;
    int  boss_spawn_time_ms_   = 60000;
    int  mission_section_point_ = 1;
    int  form_lag_time_ms_      = 1000;
    int  no_form_time_ms_       = 0;
    int  no_form_type_          = -10;
    int  no_form_freq_ms_       = 0;

    bool is_shotting_            = false;
    bool is_spelling_            = false;
    bool is_playsound_           = false;
    bool is_game_over_           = false;
    bool is_game_finish_         = false;
    bool is_replay_mode_         = false;
    bool show_small_window_      = false;
    bool is_online_mode_         = false;

    int   gameover_time_ms_         = 0;
    float gameover_bg_alpha_        = 0.0f;
    float gameover_word_alpha_      = 0.0f;
    float tip_alpha_                = 0.0f;
    bool  gameover_transition_sent_ = false;
    bool  boss_warning_sound_played_ = false;
    float shot_x_        = 0.0f;
    float shot_y_        = 0.0f;
    int   selected_magic_slot_ = -1;
    bool  textures_warm_ = false;
    uint64_t next_monster_uid_ = 1;

    SpellSpec fire_spell_      {30, 120,  70.0f};
    SpellSpec ice_spell_       {35,  95,  85.0f};
    SpellSpec lightning_spell_ {45, 180, 120.0f};
};

}
