#ifndef __RASTERIZER_DX11_DEFINE_FX_CONSTANTS_H
#define __RASTERIZER_DX11_DEFINE_FX_CONSTANTS_H

enum
{
	k_viewproj_xform = 2621440,
	k_view_xform = 2621444,
	k_screen_xform = 2621448,
	k_viewport_scale = 2621450,
	k_viewport_scale_pad = -1071120374,
	k_viewport_offset = 2621451,
	k_viewport_offset_pad = -1071120373,
	k_viewproj_xform_x = 2621440,
	k_viewproj_xform_y = 2621441,
	k_viewproj_xform_z = 2621442,
	k_viewproj_xform_w = 2621443,
	k_camera_forward = 2621444,
	k_camera_left = 2621445,
	k_camera_up = 2621446,
	k_camera_position = 2621447,
	k_screen_xform_x = 2621448,
	k_screen_xform_y = 2621449,
	k_vs_exposure = 2686976,
	k_vs_alt_exposure = 2686977,
	k_vs_atmosphere_constant_0 = 2752512,
	k_vs_atmosphere_constant_1 = 2752513,
	k_vs_atmosphere_constant_2 = 2752514,
	k_vs_atmosphere_constant_3 = 2752515,
	k_vs_atmosphere_constant_4 = 2752516,
	k_vs_atmosphere_constant_5 = 2752517,
	k_vs_atmosphere_constant_extra = 2752518,
	k_vs_lighting_constant_0 = 2818048,
	k_vs_lighting_constant_1 = 2818049,
	k_vs_lighting_constant_2 = 2818050,
	k_vs_lighting_constant_3 = 2818051,
	k_vs_lighting_constant_4 = 2818052,
	k_vs_lighting_constant_5 = 2818053,
	k_vs_lighting_constant_6 = 2818054,
	k_vs_lighting_constant_7 = 2818055,
	k_vs_lighting_constant_8 = 2818056,
	k_vs_lighting_constant_9 = 2818057,
	k_vs_shadow_projection = 2883584,
	k_register_camera_position_ps = 2949120,
	k_ps_exposure = 3014656,
	k_ps_alt_exposure = 3014657,
	k_ps_lighting_constant_0 = 3080192,
	k_ps_lighting_constant_1 = 3080193,
	k_ps_lighting_constant_2 = 3080194,
	k_ps_lighting_constant_3 = 3080195,
	k_ps_lighting_constant_4 = 3080196,
	k_ps_lighting_constant_5 = 3080197,
	k_ps_lighting_constant_6 = 3080198,
	k_ps_lighting_constant_7 = 3080199,
	k_ps_lighting_constant_8 = 3080200,
	k_ps_lighting_constant_9 = 3080201,
	k_ps_dynamic_light_gel_xform = 3080192,
	k_ps_texture_size = 3145728,
	k_ps_texture_size_pad = -2144337920,
	k_ps_dynamic_environment_blend = 3145729,
	k_ps_render_debug_mode = 3145730,
	k_shader_pc_specular_enabled = 3145731,
	k_shader_pc_specular_enabled_pad = 1076887555,
	k_shader_pc_albedo_lighting = 3145732,
	k_shader_pc_albedo_lighting_pad = 1076887556,
	k_ps_ldr_gamma2 = 3145733,
	k_ps_hdr_gamma2 = 1076887557,
	k_ps_actually_calc_albedo = -2144337915,
	k_ps_lightmap_compress_constant_using_dxt = -1070596091,
	k_ps_lightmap_compress_constant_0 = 3211264,
	k_ps_lightmap_compress_constant_1 = 3211265,
	k_ps_lightmap_compress_constant_2 = 3211266,
	k_register_simple_light_count = 3276800,
	k_register_simple_light_count_pad = 1077018624,
	k_register_simple_light_start = 3276801,
	k_vs_sampler_atmosphere_neta_table = 67108864,
	k_vs_sampler_weather_occlusion = 67108865,
	k_sampler_lightprobe_texture_array = 13,
	k_sampler_dominant_light_intensity_map = 14,
	k_sampler_scene_ldr_texture = 15,
	k_sampler_albedo_texture = 16,
	k_sampler_normal_texture = 17,
	k_sampler_depth_buffer = 18,
};
enum
{
	k_vs_decal_sprite = 1245184,
	k_vs_decal_pixel_kill_enabled = 1245185,
	k_ps_decal_fade = 1310720,
	k_ps_decal_fade_pad = 1075052544,
};
enum
{
	k_postprocess_pixel_size = 4390912,
	k_postprocess_scale = 4390913,
	k_postprocess_hue_saturation_matrix = 4390914,
	k_postprocess_contrast = 4390917,
};
enum
{
	k_vs_wind_data = 5767168,
	k_vs_wind_data2 = 5767169,
};
enum
{
	k_ps_patchy_fog_inverse_z_transform = 4194304,
	k_ps_patchy_fog_texcoord_basis = 4194305,
	k_ps_patchy_fog_attenuation_data = 4194306,
	k_ps_patchy_fog_eye_position = 4194307,
	k_ps_patchy_fog_window_pixel_bounds = 4194308,
	k_ps_patchy_fog_atmosphere_constant_0 = 4194309,
	k_ps_patchy_fog_atmosphere_constant_1 = 4194310,
	k_ps_patchy_fog_atmosphere_constant_2 = 4194311,
	k_ps_patchy_fog_atmosphere_constant_3 = 4194312,
	k_ps_patchy_fog_atmosphere_constant_4 = 4194313,
	k_ps_patchy_fog_atmosphere_constant_5 = 4194314,
	k_ps_patchy_fog_atmosphere_constant_extra = 4194315,
	k_ps_patchy_fog_sheet_fade_factors0 = 4259840,
	k_ps_patchy_fog_sheet_fade_factors1 = 4259841,
	k_ps_patchy_fog_sheet_depths0 = 4259842,
	k_ps_patchy_fog_sheet_depths1 = 4259843,
	k_ps_patchy_fog_tex_coord_transform0 = 4259844,
	k_ps_patchy_fog_tex_coord_transform1 = 4259845,
	k_ps_patchy_fog_tex_coord_transform2 = 4259846,
	k_ps_patchy_fog_tex_coord_transform3 = 4259847,
	k_ps_patchy_fog_tex_coord_transform4 = 4259848,
	k_ps_patchy_fog_tex_coord_transform5 = 4259849,
	k_ps_patchy_fog_tex_coord_transform6 = 4259850,
	k_ps_patchy_fog_tex_coord_transform7 = 4259851,
};
enum
{
	k_ps_crop_texcoord_xform = 983040,
	k_ps_crop_bounds = 983041,
};
enum
{
	k_ps_rotate_2d_offset = 4521984,
};
enum
{
	k_ps_apply_color_matrix_dest_red = 196608,
	k_ps_apply_color_matrix_dest_green = 196609,
	k_ps_apply_color_matrix_dest_blue = 196610,
	k_ps_apply_color_matrix_dest_alpha = 196611,
};
enum
{
	k_ps_screenshot_display_swap_color_channels = 4587520,
};
enum
{
	k_vs_implicit_hill_color = 3342336,
	k_vs_implicit_hill_z_scales = 3342337,
	k_vs_implicit_hill_transform1 = 3342338,
	k_vs_implicit_hill_transform2 = 3342339,
	k_vs_implicit_hill_transform3 = 3342340,
	k_vs_implicit_hill_use_zscales = 3342341,
};
enum
{
	k_ps_stencil_stipple_block_size = 5308416,
	k_ps_stencil_stipple_block_size_pad = 1079050240,
	k_ps_stencil_stipple_odd_bits = 5308417,
};
enum
{
	k_ps_debug_2d_fill_color = 1179648,
};
enum
{
	k_vs_lens_flare_center_rotation = 3538944,
	k_vs_lens_flare_scale = 3538945,
	k_ps_lens_flare_modulation_factor = 3604480,
	k_ps_lens_flare_tint_color = 3604481,
};
enum
{
	k_ps_displacement_screen_constants = 1703936,
	k_ps_displacement_window_bounds = 1703937,
	k_ps_displacement_current_view_projection = 1703938,
	k_ps_displacement_previous_view_projection = 1703942,
	k_ps_displacement_screen_to_world = 1703946,
	k_ps_displacement_num_taps = 1769472,
	k_ps_displacement_num_taps_pad = 1075511296,
	k_ps_displacement_misc_values = 1769473,
	k_ps_displacement_blur_max_and_scale = 1769474,
	k_ps_displacement_crosshair_center = 1769475,
	k_ps_displacement_zbuffer_xform = 1769476,
	k_ps_displacement_do_distortion = 1769477,
};
enum
{
	k_position_compression_scale = 2293760,
	k_position_compression_offset = 2293761,
	k_uv_compression_scale_offset = 2293762,
	k_node_start = 7143424,
	k_alpha_test_shader_lighting_constant = 2359296,
	k_ps_dominant_light_direction = 2424832,
	k_ps_dominant_light_intensity = 2424833,
	k_ps_constant_shadow_alpha = 2490368,
	k_ps_bool_dynamic_light_shadowing = 2490369,
	k_ps_active_camo_factor = 2555904,
	k_ps_distort_bounds = 2555905,
};
enum
{
	k_vs_particle_render_hidden_from_compiler = 3932160,
	k_vs_particle_render_local_to_world = 3932161,
	k_vs_particle_render_local_to_world_pad = -1069809660,
	k_vs_particle_render_occlusion_to_world = 3932165,
	k_vs_particle_render_world_to_occlusion = 3932168,
	k_vs_particle_render_motion_blur_state = 3932171,
	k_vs_particle_render_sprite = 3932174,
	k_vs_particle_render_state = 3932175,
	k_vs_particle_render_sprite_frames = 3932185,
	k_vs_particle_render_collision = 3932201,
	k_vs_particle_render_mesh_variants = 6553600,
	k_ps_particle_render_depth_constants = 3997696,
	k_vs_mesh_vertices = 17,
};
enum
{
	k_vs_particle_update_delta_time = 4128768,
	k_vs_particle_update_delta_time_pad = 1077870592,
	k_vs_particle_update_hidden_from_compiler = 4128769,
	k_vs_particle_update_tile_to_world = 4128770,
	k_vs_particle_update_world_to_tile = 4128773,
	k_vs_particle_update_occlusion_to_world = 4128776,
	k_vs_particle_update_world_to_occlusion = 4128779,
	k_vs_particle_update_state = 4128782,
	k_vs_particle_update_tiled = 4128785,
	k_vs_particle_update_collision = 1077870609,
	k_particle_emitter_all_properties = 5898240,
	k_particle_emitter_all_functions = 5898253,
	k_particle_emitter_all_colors = 5898353,
	k_particle_state_all_state = 5963776,
	k_cs_sampler_weather_occlusion = 33554433,
};
enum
{
	k_ps_shadow_geometry_color = 4784128,
};
enum
{
	k_ps_shadow_apply_view_inverse_matrix = 4718592,
	k_ps_shadow_apply_occlusion_spheres = 4718596,
	k_ps_shadow_apply_occlusion_spheres_count = 4718636,
};
enum
{
	k_vs_shield_impact_extrusion_distance = 4849664,
	k_ps_shield_impact_bound_sphere = 4915200,
	k_ps_shield_impact_shield_dynamic_quantities = 4915201,
	k_ps_shield_impact_texture_quantities = 4915202,
	k_ps_shield_impact_plasma1_settings = 4915203,
	k_ps_shield_impact_plasma2_settings = 4915204,
	k_ps_shield_impact_overshield_color1 = 4915205,
	k_ps_shield_impact_overshield_color1_pad = -1068826619,
	k_ps_shield_impact_overshield_color2 = 4915206,
	k_ps_shield_impact_overshield_color2_pad = -1068826618,
	k_ps_shield_impact_overshield_ambient_color = 4915207,
	k_ps_shield_impact_overshield_ambient_color_pad = -1068826617,
	k_ps_shield_impact_color1 = 4915208,
	k_ps_shield_impact_color1_pad = -1068826616,
	k_ps_shield_impact_color2 = 4915209,
	k_ps_shield_impact_color2_pad = -1068826615,
	k_ps_shield_impact_ambient_color = 4915210,
	k_ps_shield_impact_ambient_color_pad = -1068826614,
};
enum
{
	k_vs_decorators_lights = 1376256,
	k_vs_decorators_lights_count = 1376296,
	k_vs_decorators_instance_compression_offset = 1441792,
	k_vs_decorators_instance_compression_scale = 1441793,
	k_vs_decorators_instance_data = 1441794,
	k_vs_decorators_lod_constants = 1441795,
	k_vs_decorators_translucency = 1441796,
	k_vs_decorators_wave_flow = 1441797,
	k_vs_decorators_sun_direction = 1507328,
	k_vs_decorators_sun_direction_pad = -1072234496,
	k_vs_decorators_sun_color = 1507329,
	k_vs_decorators_sun_color_pad = -1072234495,
	k_vs_decorators_instance_position_and_scale = 1572864,
	k_vs_decorators_instance_quaternion = 1572865,
	k_ps_decorators_contrast = 1638400,
	k_ps_decorators_contrast_pad = -1072103424,
};
enum
{
	k_ps_water_view_depth_constant = 5439488,
	k_ps_water_is_lightmap_exist = 5439489,
	k_ps_water_is_interaction = 1079181313,
	k_ps_water_is_tessellated = -2142044159,
	k_ps_water_is_underwater = -1068302335,
	k_ps_water_tex_ripple_buffer_slope_height = 10,
	k_vs_water_tex_ripple_buffer_slope_height = 67108867,
	k_vs_water_tess_camera_position = 5505024,
	k_vs_water_tess_camera_forward = 5505025,
	k_vs_water_tess_camera_diagonal = 5505026,
	k_vs_water_camera_position = 5570560,
	k_vs_water_camera_position_pad = -1068171264,
	k_vs_water_ripple_pattern_count = 5570561,
	k_vs_water_ripple_pattern_count_pad = 1079312385,
	k_vs_water_ripple_real_frametime_ratio = 6946816,
	k_vs_water_ripple_real_frametime_ratio_pad = 1080688640,
	k_ps_water_tex_ripple_pattern = 0,
	k_ps_water_tex_ripple_buffer_height = 1,
	k_ps_water_tex_ldr_buffer = 0,
	k_ps_water_tex_depth_buffer = 1,
	k_vs_water_ripple_buffer_radius = 5636096,
	k_vs_water_ripple_buffer_radius_pad = 1079377920,
	k_vs_water_ripple_buffer_center = 5636097,
	k_vs_water_ripple_buffer_center_pad = -2141847551,
	k_ps_water_view_xform_inverse = 5701632,
	k_ps_water_player_view_constant = 5701636,
	k_ps_water_camera_position = 5701637,
	k_ps_water_underwater_murkiness = 5701638,
	k_ps_water_underwater_murkiness_pad = 1079443462,
	k_ps_water_underwater_fog_color = 5701639,
	k_ripple_index_range = 6881280,
	k_cs_ripple_buffer = 0,
	k_vs_ripple_buffer = 16,
};
enum
{
	k_ps_downsample_intensity_vector = 1835008,
};
enum
{
	k_vs_fxaa_texel_size = 2031616,
};
enum
{
	k_ps_kernel_5_kernel = 3407872,
};
enum
{
	k_ps_spike_blur_source_pixel_size = 5111808,
	k_ps_spike_blur_source_pixel_size_pad = -2142371840,
	k_ps_spike_blur_offset_delta = 5111809,
	k_ps_spike_blur_initial_color = 5111810,
	k_ps_spike_blur_initial_color_pad = -1068630014,
	k_ps_spike_blur_delta_color = 5111811,
	k_ps_spike_blur_delta_color_pad = -1068630013,
};
enum
{
	k_ps_final_composite_intensity = 1900544,
	k_ps_final_composite_tone_curve_constants = 1900545,
	k_ps_final_composite_player_window_constants = 1900546,
	k_ps_final_composite_bloom_sampler_xform = 1900547,
	k_ps_final_composite_cg_blend_factor = 1900548,
	k_ps_final_composite_depth_constants = 1966080,
	k_ps_final_composite_depth_constants2 = 1966081,
};
enum
{
	k_vs_ssao_texcoord_scale = 5177344,
	k_vs_ssao_frustum_scale = 5177345,
	k_ps_ssao_params = 5242880,
	k_ps_ssao_frustum_scale = 5242881,
	k_ps_ssao_mv_1 = 5242882,
	k_ps_ssao_mv_2 = 5242883,
	k_ps_ssao_mv_3 = 5242884,
};
enum
{
	k_ps_lightshafts_tint = 3670016,
	k_ps_lightshafts_sun_pos = 3670017,
	k_ps_lightshafts_inner_size = 3670018,
};
enum
{
	k_ps_radial_blur_tint = 4456448,
	k_ps_radial_blur_center_scale = 4456449,
	k_ps_radial_blur_weights0 = 4456450,
	k_ps_radial_blur_weights1 = 4456451,
	k_ps_radial_blur_weights2 = 4456452,
	k_ps_radial_blur_weights3 = 4456453,
};
enum
{
	k_beam_all_properties = 6029312,
	k_beam_all_functions = 6029320,
	k_beam_all_colors = 6029420,
	k_beam_state_all_state = 6094848,
	k_beam_strip = 6160384,
	k_beam_index_range = 6619136,
	k_cs_beam_profile_state_buffer = 0,
	k_vs_beam_profile_state_buffer = 16,
};
enum
{
	k_contrail_state_all_state = 6291456,
	k_contrail_index_range = 6684672,
	k_cs_contrail_profile_state_buffer = 0,
	k_vs_contrail_profile_state_buffer = 16,
};
enum
{
	k_cs_contrail_address_buffer = 4,
	k_cs_contrail_profile_state_spawn_buffer = 5,
};
enum
{
	k_contrail_all_properties = 6225920,
	k_contrail_all_functions = 6225931,
	k_contrail_all_colors = 6226031,
	k_vs_contrail_update_delta_time = 851968,
};
enum
{
	k_contrail_strip = 6750208,
};
enum
{
	k_light_volume_all_properties = 6356992,
	k_light_volume_all_functions = 6356996,
	k_light_volume_all_colors = 6357096,
	k_light_volume_state_all_state = 6422528,
	k_light_volume_strip = 6488064,
	k_light_volume_index_range = 6815744,
	k_cs_light_volume_profile_state_buffer = 0,
	k_vs_light_volume_profile_state_buffer = 16,
};
enum
{
	k_cs_particle_address_buffer = 4,
	k_cs_particle_state_spawn_buffer = 5,
};
enum
{
	k_particle_index_range = 3866624,
};
enum
{
	k_cs_particle_state_buffer = 0,
	k_vs_particle_state_buffer = 16,
};
enum
{
	k_vs_chud_widget_offset = 393216,
	k_vs_chud_widget_transform1 = 393217,
	k_vs_chud_widget_transform2 = 393218,
	k_vs_chud_widget_transform3 = 393219,
	k_vs_chud_screen_size = 393220,
	k_vs_chud_basis_0 = 393221,
	k_vs_chud_basis_1 = 393222,
	k_vs_chud_basis_2 = 393223,
	k_vs_chud_basis_3 = 393224,
	k_vs_chud_screen_scale_and_offset = 393225,
	k_vs_chud_project_scale_and_offset = 393226,
	k_vs_chud_widget_mirror = 393227,
	k_vs_chud_screenshot_info = 393228,
	k_vs_chud_texture_transform = 393229,
	k_vs_chud_cortana_vertex = 393230,
	k_ps_chud_color_output_A = 458752,
	k_ps_chud_color_output_B = 458753,
	k_ps_chud_color_output_C = 458754,
	k_ps_chud_color_output_D = 458755,
	k_ps_chud_color_output_E = 458756,
	k_ps_chud_color_output_F = 458757,
	k_ps_chud_scalar_output_ABCD = 458758,
	k_ps_chud_scalar_output_EF = 458759,
	k_ps_chud_texture_bounds = 458760,
	k_ps_chud_savedfilm_chap1 = 458761,
	k_ps_chud_savedfilm_chap2 = 458762,
	k_ps_chud_savedfilm_chap3 = 458763,
	k_ps_chud_savedfilm_data = 458764,
	k_ps_chud_cortana_pixel = 458765,
	k_ps_chud_comp_colorize_enabled = 1074200589,
	k_ps_chud_widget_transform1 = 524288,
	k_ps_chud_widget_transform2 = 524289,
	k_ps_chud_widget_transform3 = 524290,
	k_ps_chud_widget_mirror = 524291,
	k_ps_chud_screen_flash0_color = 589824,
	k_ps_chud_screen_flash0_data = 589825,
	k_ps_chud_screen_flash1_color = 589826,
	k_ps_chud_screen_flash1_data = 589827,
	k_ps_chud_screen_flash2_color = 589828,
	k_ps_chud_screen_flash2_data = 589829,
	k_ps_chud_screen_flash3_color = 589830,
	k_ps_chud_screen_flash3_data = 589831,
	k_ps_chud_screen_flash_center = 589832,
	k_ps_cortana_back_colormix_result = 655360,
	k_ps_cortana_back_hsv_result = 655361,
	k_ps_cortana_texcam_colormix_result = 655362,
	k_ps_cortana_comp_solarize_inmix = 655363,
	k_ps_cortana_comp_solarize_outmix = 655364,
	k_ps_cortana_comp_solarize_result = 655365,
	k_ps_cortana_comp_doubling_inmix = 655366,
	k_ps_cortana_comp_doubling_outmix = 655367,
	k_ps_cortana_comp_doubling_result = 655368,
	k_ps_cortana_comp_colorize_inmix = 655369,
	k_ps_cortana_comp_colorize_outmix = 655370,
	k_ps_cortana_comp_colorize_result = 655371,
	k_ps_cortana_texcam_bloom_inmix = 655372,
	k_ps_cortana_texcam_bloom_outmix = 655373,
	k_ps_cortana_texcam_bloom_result = 655374,
	k_ps_cortana_vignette_data = 655375,
};
enum
{
	k_ps_emblem_color_background_argb = 4325376,
	k_ps_emblem_color_icon1_argb = 4325377,
	k_ps_emblem_color_icon2_argb = 4325378,
};
enum
{
	k_viewproj_xform = 2555904,
	k_view_xform = 2555908,
	k_screen_xform = 2555912,
	k_viewport_scale = 2555914,
	k_viewport_scale_pad = -1071185910,
	k_viewport_offset = 2555915,
	k_viewport_offset_pad = -1071185909,
	k_viewproj_xform_x = 2555904,
	k_viewproj_xform_y = 2555905,
	k_viewproj_xform_z = 2555906,
	k_viewproj_xform_w = 2555907,
	k_camera_forward = 2555908,
	k_camera_left = 2555909,
	k_camera_up = 2555910,
	k_camera_position = 2555911,
	k_screen_xform_x = 2555912,
	k_screen_xform_y = 2555913,
	k_vs_exposure = 2621440,
	k_vs_alt_exposure = 2621441,
	k_vs_atmosphere_constant_0 = 2686976,
	k_vs_atmosphere_constant_1 = 2686977,
	k_vs_atmosphere_constant_2 = 2686978,
	k_vs_atmosphere_constant_3 = 2686979,
	k_vs_atmosphere_constant_4 = 2686980,
	k_vs_atmosphere_constant_5 = 2686981,
	k_vs_atmosphere_constant_extra = 2686982,
	k_vs_lighting_constant_0 = 2752512,
	k_vs_lighting_constant_1 = 2752513,
	k_vs_lighting_constant_2 = 2752514,
	k_vs_lighting_constant_3 = 2752515,
	k_vs_lighting_constant_4 = 2752516,
	k_vs_lighting_constant_5 = 2752517,
	k_vs_lighting_constant_6 = 2752518,
	k_vs_lighting_constant_7 = 2752519,
	k_vs_lighting_constant_8 = 2752520,
	k_vs_lighting_constant_9 = 2752521,
	k_vs_shadow_projection = 2818048,
	k_register_camera_position_ps = 2883584,
	k_ps_exposure = 2949120,
	k_ps_alt_exposure = 2949121,
	k_ps_lighting_constant_0 = 3014656,
	k_ps_lighting_constant_1 = 3014657,
	k_ps_lighting_constant_2 = 3014658,
	k_ps_lighting_constant_3 = 3014659,
	k_ps_lighting_constant_4 = 3014660,
	k_ps_lighting_constant_5 = 3014661,
	k_ps_lighting_constant_6 = 3014662,
	k_ps_lighting_constant_7 = 3014663,
	k_ps_lighting_constant_8 = 3014664,
	k_ps_lighting_constant_9 = 3014665,
	k_ps_dynamic_light_gel_xform = 3014656,
	k_ps_texture_size = 3080192,
	k_ps_texture_size_pad = -2144403456,
	k_ps_dynamic_environment_blend = 3080193,
	k_ps_render_debug_mode = 3080194,
	k_shader_pc_specular_enabled = 3080195,
	k_shader_pc_specular_enabled_pad = 1076822019,
	k_shader_pc_albedo_lighting = 3080196,
	k_shader_pc_albedo_lighting_pad = 1076822020,
	k_ps_ldr_gamma2 = 3080197,
	k_ps_hdr_gamma2 = 1076822021,
	k_ps_actually_calc_albedo = -2144403451,
	k_ps_lightmap_compress_constant_using_dxt = -1070661627,
	k_ps_lightmap_compress_constant_0 = 3145728,
	k_ps_lightmap_compress_constant_1 = 3145729,
	k_ps_lightmap_compress_constant_2 = 3145730,
	k_register_simple_light_count = 3211264,
	k_register_simple_light_count_pad = 1076953088,
	k_register_simple_light_start = 3211265,
	k_vs_sampler_atmosphere_neta_table = 67108864,
	k_vs_sampler_weather_occlusion = 67108865,
	k_sampler_lightprobe_texture_array = 13,
	k_sampler_dominant_light_intensity_map = 14,
	k_sampler_scene_ldr_texture = 15,
	k_sampler_albedo_texture = 16,
	k_sampler_normal_texture = 17,
	k_sampler_depth_buffer = 18,
};
enum
{
	k_postprocess_pixel_size = 4325376,
	k_postprocess_scale = 4325377,
	k_postprocess_hue_saturation_matrix = 4325378,
	k_postprocess_contrast = 4325381,
};
enum
{
	k_vs_wind_data = 5701632,
	k_vs_wind_data2 = 5701633,
};
enum
{
	k_ps_patchy_fog_inverse_z_transform = 4128768,
	k_ps_patchy_fog_texcoord_basis = 4128769,
	k_ps_patchy_fog_attenuation_data = 4128770,
	k_ps_patchy_fog_eye_position = 4128771,
	k_ps_patchy_fog_window_pixel_bounds = 4128772,
	k_ps_patchy_fog_atmosphere_constant_0 = 4128773,
	k_ps_patchy_fog_atmosphere_constant_1 = 4128774,
	k_ps_patchy_fog_atmosphere_constant_2 = 4128775,
	k_ps_patchy_fog_atmosphere_constant_3 = 4128776,
	k_ps_patchy_fog_atmosphere_constant_4 = 4128777,
	k_ps_patchy_fog_atmosphere_constant_5 = 4128778,
	k_ps_patchy_fog_atmosphere_constant_extra = 4128779,
	k_ps_patchy_fog_sheet_fade_factors0 = 4194304,
	k_ps_patchy_fog_sheet_fade_factors1 = 4194305,
	k_ps_patchy_fog_sheet_depths0 = 4194306,
	k_ps_patchy_fog_sheet_depths1 = 4194307,
	k_ps_patchy_fog_tex_coord_transform0 = 4194308,
	k_ps_patchy_fog_tex_coord_transform1 = 4194309,
	k_ps_patchy_fog_tex_coord_transform2 = 4194310,
	k_ps_patchy_fog_tex_coord_transform3 = 4194311,
	k_ps_patchy_fog_tex_coord_transform4 = 4194312,
	k_ps_patchy_fog_tex_coord_transform5 = 4194313,
	k_ps_patchy_fog_tex_coord_transform6 = 4194314,
	k_ps_patchy_fog_tex_coord_transform7 = 4194315,
};
enum
{
	k_ps_rotate_2d_offset = 4456448,
};
enum
{
	k_ps_screenshot_display_swap_color_channels = 4521984,
};
enum
{
	k_vs_implicit_hill_color = 3276800,
	k_vs_implicit_hill_z_scales = 3276801,
	k_vs_implicit_hill_transform1 = 3276802,
	k_vs_implicit_hill_transform2 = 3276803,
	k_vs_implicit_hill_transform3 = 3276804,
	k_vs_implicit_hill_use_zscales = 3276805,
};
enum
{
	k_ps_stencil_stipple_block_size = 5242880,
	k_ps_stencil_stipple_block_size_pad = 1078984704,
	k_ps_stencil_stipple_odd_bits = 5242881,
};
enum
{
	k_vs_particle_render_hidden_from_compiler = 3866624,
	k_vs_particle_render_local_to_world = 3866625,
	k_vs_particle_render_local_to_world_pad = -1069875196,
	k_vs_particle_render_occlusion_to_world = 3866629,
	k_vs_particle_render_world_to_occlusion = 3866632,
	k_vs_particle_render_motion_blur_state = 3866635,
	k_vs_particle_render_sprite = 3866638,
	k_vs_particle_render_state = 3866639,
	k_vs_particle_render_sprite_frames = 3866649,
	k_vs_particle_render_collision = 3866665,
	k_vs_particle_render_mesh_variants = 6488064,
	k_ps_particle_render_depth_constants = 3932160,
	k_vs_mesh_vertices = 17,
};
enum
{
	k_vs_lens_flare_center_rotation = 3473408,
	k_vs_lens_flare_scale = 3473409,
	k_ps_lens_flare_modulation_factor = 3538944,
	k_ps_lens_flare_tint_color = 3538945,
};
enum
{
	k_position_compression_scale = 2228224,
	k_position_compression_offset = 2228225,
	k_uv_compression_scale_offset = 2228226,
	k_node_start = 7077888,
	k_alpha_test_shader_lighting_constant = 2293760,
	k_ps_dominant_light_direction = 2359296,
	k_ps_dominant_light_intensity = 2359297,
	k_ps_constant_shadow_alpha = 2424832,
	k_ps_bool_dynamic_light_shadowing = 2424833,
	k_ps_active_camo_factor = 2490368,
	k_ps_distort_bounds = 2490369,
};
enum
{
	k_vs_particle_update_delta_time = 4063232,
	k_vs_particle_update_delta_time_pad = 1077805056,
	k_vs_particle_update_hidden_from_compiler = 4063233,
	k_vs_particle_update_tile_to_world = 4063234,
	k_vs_particle_update_world_to_tile = 4063237,
	k_vs_particle_update_occlusion_to_world = 4063240,
	k_vs_particle_update_world_to_occlusion = 4063243,
	k_vs_particle_update_state = 4063246,
	k_vs_particle_update_tiled = 4063249,
	k_vs_particle_update_collision = 1077805073,
	k_particle_emitter_all_properties = 5832704,
	k_particle_emitter_all_functions = 5832717,
	k_particle_emitter_all_colors = 5832817,
	k_particle_state_all_state = 5898240,
	k_cs_sampler_weather_occlusion = 33554433,
};
enum
{
	k_ps_shadow_geometry_color = 4718592,
};
enum
{
	k_ps_shadow_apply_view_inverse_matrix = 4653056,
	k_ps_shadow_apply_occlusion_spheres = 4653060,
	k_ps_shadow_apply_occlusion_spheres_count = 4653100,
};
enum
{
	k_vs_shield_impact_extrusion_distance = 4784128,
	k_ps_shield_impact_bound_sphere = 4849664,
	k_ps_shield_impact_shield_dynamic_quantities = 4849665,
	k_ps_shield_impact_texture_quantities = 4849666,
	k_ps_shield_impact_plasma1_settings = 4849667,
	k_ps_shield_impact_plasma2_settings = 4849668,
	k_ps_shield_impact_overshield_color1 = 4849669,
	k_ps_shield_impact_overshield_color1_pad = -1068892155,
	k_ps_shield_impact_overshield_color2 = 4849670,
	k_ps_shield_impact_overshield_color2_pad = -1068892154,
	k_ps_shield_impact_overshield_ambient_color = 4849671,
	k_ps_shield_impact_overshield_ambient_color_pad = -1068892153,
	k_ps_shield_impact_color1 = 4849672,
	k_ps_shield_impact_color1_pad = -1068892152,
	k_ps_shield_impact_color2 = 4849673,
	k_ps_shield_impact_color2_pad = -1068892151,
	k_ps_shield_impact_ambient_color = 4849674,
	k_ps_shield_impact_ambient_color_pad = -1068892150,
};
enum
{
	k_ps_water_view_depth_constant = 5373952,
	k_ps_water_is_lightmap_exist = 5373953,
	k_ps_water_is_interaction = 1079115777,
	k_ps_water_is_tessellated = -2142109695,
	k_ps_water_is_underwater = -1068367871,
	k_ps_water_tex_ripple_buffer_slope_height = 10,
	k_vs_water_tex_ripple_buffer_slope_height = 67108867,
	k_vs_water_tess_camera_position = 5439488,
	k_vs_water_tess_camera_forward = 5439489,
	k_vs_water_tess_camera_diagonal = 5439490,
	k_vs_water_camera_position = 5505024,
	k_vs_water_camera_position_pad = -1068236800,
	k_vs_water_ripple_pattern_count = 5505025,
	k_vs_water_ripple_pattern_count_pad = 1079246849,
	k_vs_water_ripple_real_frametime_ratio = 6881280,
	k_vs_water_ripple_real_frametime_ratio_pad = 1080623104,
	k_ps_water_tex_ripple_pattern = 0,
	k_ps_water_tex_ripple_buffer_height = 1,
	k_ps_water_tex_ldr_buffer = 0,
	k_ps_water_tex_depth_buffer = 1,
	k_vs_water_ripple_buffer_radius = 5570560,
	k_vs_water_ripple_buffer_radius_pad = 1079312384,
	k_vs_water_ripple_buffer_center = 5570561,
	k_vs_water_ripple_buffer_center_pad = -2141913087,
	k_ps_water_view_xform_inverse = 5636096,
	k_ps_water_player_view_constant = 5636100,
	k_ps_water_camera_position = 5636101,
	k_ps_water_underwater_murkiness = 5636102,
	k_ps_water_underwater_murkiness_pad = 1079377926,
	k_ps_water_underwater_fog_color = 5636103,
	k_ripple_index_range = 6815744,
	k_cs_ripple_buffer = 0,
	k_vs_ripple_buffer = 16,
};
enum
{
	k_ps_radial_blur_tint = 4390912,
	k_ps_radial_blur_center_scale = 4390913,
	k_ps_radial_blur_weights0 = 4390914,
	k_ps_radial_blur_weights1 = 4390915,
	k_ps_radial_blur_weights2 = 4390916,
	k_ps_radial_blur_weights3 = 4390917,
};
enum
{
	k_ps_kernel_5_kernel = 3342336,
};
enum
{
	k_ps_spike_blur_source_pixel_size = 5046272,
	k_ps_spike_blur_source_pixel_size_pad = -2142437376,
	k_ps_spike_blur_offset_delta = 5046273,
	k_ps_spike_blur_initial_color = 5046274,
	k_ps_spike_blur_initial_color_pad = -1068695550,
	k_ps_spike_blur_delta_color = 5046275,
	k_ps_spike_blur_delta_color_pad = -1068695549,
};
enum
{
	k_vs_ssao_texcoord_scale = 5111808,
	k_vs_ssao_frustum_scale = 5111809,
	k_ps_ssao_params = 5177344,
	k_ps_ssao_frustum_scale = 5177345,
	k_ps_ssao_mv_1 = 5177346,
	k_ps_ssao_mv_2 = 5177347,
	k_ps_ssao_mv_3 = 5177348,
};
enum
{
	k_ps_lightshafts_tint = 3604480,
	k_ps_lightshafts_sun_pos = 3604481,
	k_ps_lightshafts_inner_size = 3604482,
};
enum
{
	k_beam_all_properties = 5963776,
	k_beam_all_functions = 5963784,
	k_beam_all_colors = 5963884,
	k_beam_state_all_state = 6029312,
	k_beam_strip = 6094848,
	k_beam_index_range = 6553600,
	k_cs_beam_profile_state_buffer = 0,
	k_vs_beam_profile_state_buffer = 16,
};
enum
{
	k_contrail_strip = 6684672,
};
enum
{
	k_contrail_state_all_state = 6225920,
	k_contrail_index_range = 6619136,
	k_cs_contrail_profile_state_buffer = 0,
	k_vs_contrail_profile_state_buffer = 16,
};
enum
{
	k_contrail_all_properties = 6160384,
	k_contrail_all_functions = 6160395,
	k_contrail_all_colors = 6160495,
	k_vs_contrail_update_delta_time = 851968,
};
enum
{
	k_light_volume_all_properties = 6291456,
	k_light_volume_all_functions = 6291460,
	k_light_volume_all_colors = 6291560,
	k_light_volume_state_all_state = 6356992,
	k_light_volume_strip = 6422528,
	k_light_volume_index_range = 6750208,
	k_cs_light_volume_profile_state_buffer = 0,
	k_vs_light_volume_profile_state_buffer = 16,
};
enum
{
	k_particle_index_range = 3801088,
};
enum
{
	k_ps_emblem_color_background_argb = 4259840,
	k_ps_emblem_color_icon1_argb = 4259841,
	k_ps_emblem_color_icon2_argb = 4259842,
};
enum
{
	k_viewproj_xform = 2555904,
	k_view_xform = 2555908,
	k_screen_xform = 2555912,
	k_vs_always_true = 2555914,
	k_viewproj_xform_x = 2555904,
	k_viewproj_xform_y = 2555905,
	k_viewproj_xform_z = 2555906,
	k_viewproj_xform_w = 2555907,
	k_camera_forward = 2555908,
	k_camera_left = 2555909,
	k_camera_up = 2555910,
	k_camera_position = 2555911,
	k_screen_xform_x = 2555912,
	k_screen_xform_y = 2555913,
	k_vs_exposure = 2621440,
	k_vs_alt_exposure = 2621441,
	k_vs_atmosphere_constant_0 = 2686976,
	k_vs_atmosphere_constant_1 = 2686977,
	k_vs_atmosphere_constant_2 = 2686978,
	k_vs_atmosphere_constant_3 = 2686979,
	k_vs_atmosphere_constant_4 = 2686980,
	k_vs_atmosphere_constant_5 = 2686981,
	k_vs_atmosphere_constant_extra = 2686982,
	k_vs_lighting_constant_0 = 2752512,
	k_vs_lighting_constant_1 = 2752513,
	k_vs_lighting_constant_2 = 2752514,
	k_vs_lighting_constant_3 = 2752515,
	k_vs_lighting_constant_4 = 2752516,
	k_vs_lighting_constant_5 = 2752517,
	k_vs_lighting_constant_6 = 2752518,
	k_vs_lighting_constant_7 = 2752519,
	k_vs_lighting_constant_8 = 2752520,
	k_vs_lighting_constant_9 = 2752521,
	k_vs_shadow_projection = 2818048,
	k_register_camera_position_ps = 2883584,
	k_ps_exposure = 2949120,
	k_ps_alt_exposure = 2949121,
	k_ps_lighting_constant_0 = 3014656,
	k_ps_lighting_constant_1 = 3014657,
	k_ps_lighting_constant_2 = 3014658,
	k_ps_lighting_constant_3 = 3014659,
	k_ps_lighting_constant_4 = 3014660,
	k_ps_lighting_constant_5 = 3014661,
	k_ps_lighting_constant_6 = 3014662,
	k_ps_lighting_constant_7 = 3014663,
	k_ps_lighting_constant_8 = 3014664,
	k_ps_lighting_constant_9 = 3014665,
	k_ps_dynamic_light_gel_xform = 3014656,
	k_ps_texture_size = 3080192,
	k_ps_texture_size_pad = -2144403456,
	k_ps_dynamic_environment_blend = 3080193,
	k_ps_render_debug_mode = 3080194,
	k_shader_pc_specular_enabled = 3080195,
	k_shader_pc_specular_enabled_pad = 1076822019,
	k_shader_pc_albedo_lighting = 3080196,
	k_shader_pc_albedo_lighting_pad = 1076822020,
	k_ps_ldr_gamma2 = 3080197,
	k_ps_hdr_gamma2 = 1076822021,
	k_ps_actually_calc_albedo = -2144403451,
	k_ps_lightmap_compress_constant_using_dxt = -1070661627,
	k_ps_lightmap_compress_constant_0 = 3145728,
	k_ps_lightmap_compress_constant_1 = 3145729,
	k_ps_lightmap_compress_constant_2 = 3145730,
	k_register_simple_light_count = 3211264,
	k_register_simple_light_count_pad = 1076953088,
	k_register_simple_light_start = 3211265,
	k_vs_sampler_atmosphere_neta_table = 67108864,
	k_vs_sampler_weather_occlusion = 67108865,
	k_sampler_lightprobe_texture_array = 13,
	k_sampler_dominant_light_intensity_map = 14,
	k_sampler_scene_ldr_texture = 15,
	k_sampler_albedo_texture = 16,
	k_sampler_normal_texture = 17,
	k_sampler_depth_buffer = 18,
};
enum
{
	k_vs_particle_render_hidden_from_compiler = 3866624,
	k_vs_particle_render_local_to_world = 3866625,
	k_vs_particle_render_local_to_world_pad = -1069875196,
	k_vs_particle_render_motion_blur_state = 3866629,
	k_vs_particle_render_sprite = 3866632,
	k_vs_particle_render_state = 3866633,
	k_vs_particle_render_sprite_frames = 3866643,
	k_vs_particle_render_collision = 3866659,
	k_vs_particle_render_mesh_variants = 6488064,
	k_ps_particle_render_depth_constants = 3932160,
	k_vs_mesh_vertices = 17,
};

enum
{
	k_viewproj_xform = 2555904,
	k_view_xform = 2555908,
	k_screen_xform = 2555912,
	k_clip_plane = 2555914,
	k_vs_always_true = 2555915,
	k_viewproj_xform_x = 2555904,
	k_viewproj_xform_y = 2555905,
	k_viewproj_xform_z = 2555906,
	k_viewproj_xform_w = 2555907,
	k_camera_forward = 2555908,
	k_camera_left = 2555909,
	k_camera_up = 2555910,
	k_camera_position = 2555911,
	k_screen_xform_x = 2555912,
	k_screen_xform_y = 2555913,
	k_vs_exposure = 2621440,
	k_vs_alt_exposure = 2621441,
	k_vs_atmosphere_constant_0 = 2686976,
	k_vs_atmosphere_constant_1 = 2686977,
	k_vs_atmosphere_constant_2 = 2686978,
	k_vs_atmosphere_constant_3 = 2686979,
	k_vs_atmosphere_constant_4 = 2686980,
	k_vs_atmosphere_constant_5 = 2686981,
	k_vs_atmosphere_constant_extra = 2686982,
	k_vs_lighting_constant_0 = 2752512,
	k_vs_lighting_constant_1 = 2752513,
	k_vs_lighting_constant_2 = 2752514,
	k_vs_lighting_constant_3 = 2752515,
	k_vs_lighting_constant_4 = 2752516,
	k_vs_lighting_constant_5 = 2752517,
	k_vs_lighting_constant_6 = 2752518,
	k_vs_lighting_constant_7 = 2752519,
	k_vs_lighting_constant_8 = 2752520,
	k_vs_lighting_constant_9 = 2752521,
	k_vs_dynamic_light_clip_planes = 6946816,
	k_vs_shadow_projection = 2818048,
	k_register_camera_position_ps = 2883584,
	k_ps_exposure = 2949120,
	k_ps_alt_exposure = 2949121,
	k_ps_lighting_constant_0 = 3014656,
	k_ps_lighting_constant_1 = 3014657,
	k_ps_lighting_constant_2 = 3014658,
	k_ps_lighting_constant_3 = 3014659,
	k_ps_lighting_constant_4 = 3014660,
	k_ps_lighting_constant_5 = 3014661,
	k_ps_lighting_constant_6 = 3014662,
	k_ps_lighting_constant_7 = 3014663,
	k_ps_lighting_constant_8 = 3014664,
	k_ps_lighting_constant_9 = 3014665,
	k_ps_dynamic_light_gel_xform = 3014656,
	k_ps_texture_size = 3080192,
	k_ps_texture_size_pad = -2144403456,
	k_ps_dynamic_environment_blend = 3080193,
	k_ps_render_debug_mode = 3080194,
	k_shader_pc_specular_enabled = 3080195,
	k_shader_pc_specular_enabled_pad = 1076822019,
	k_shader_pc_albedo_lighting = 3080196,
	k_shader_pc_albedo_lighting_pad = 1076822020,
	k_ps_ldr_gamma2 = 3080197,
	k_ps_hdr_gamma2 = 1076822021,
	k_ps_actually_calc_albedo = -2144403451,
	k_ps_lightmap_compress_constant_using_dxt = -1070661627,
	k_ps_lightmap_compress_constant_0 = 3145728,
	k_ps_lightmap_compress_constant_1 = 3145729,
	k_ps_lightmap_compress_constant_2 = 3145730,
	k_register_simple_light_count = 3211264,
	k_register_simple_light_count_pad = 1076953088,
	k_register_simple_light_start = 3211265,
	k_vs_sampler_atmosphere_neta_table = 67108864,
	k_vs_sampler_weather_occlusion = 67108865,
	k_sampler_lightprobe_texture_array = 13,
	k_sampler_dominant_light_intensity_map = 14,
	k_sampler_scene_ldr_texture = 15,
	k_sampler_albedo_texture = 16,
	k_sampler_normal_texture = 17,
	k_sampler_depth_buffer = 18,
};

enum
{
	k_position_compression_scale = 2228224,
	k_position_compression_offset = 2228225,
	k_uv_compression_scale_offset = 2228226,
	k_node_start = 7143424,
	k_alpha_test_shader_lighting_constant = 2293760,
	k_ps_dominant_light_direction = 2359296,
	k_ps_dominant_light_intensity = 2359297,
	k_ps_constant_shadow_alpha = 2424832,
	k_ps_bool_dynamic_light_shadowing = 2424833,
	k_ps_active_camo_factor = 2490368,
	k_ps_distort_bounds = 2490369,
};

#endif // __RASTERIZER_DX11_DEFINE_FX_CONSTANTS_H
