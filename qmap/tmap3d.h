
extern int compute_mip_level(int face);
extern void get_face_extent(int face, int *u0, int *v0, int *u1, int *v1);
extern void compute_texture_gradients(int face, int tex, int mip,
             float u, float v);
