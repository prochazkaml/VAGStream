#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBGS.H>

// Camera coordinates
typedef struct {
	int		x, y, z;
	int		pan, til, rol;
	VECTOR	pos;
	SVECTOR rot;
	GsRVIEW2 view;
	GsCOORDINATE2 coord2;
} Camera_t;

extern Camera_t Camera;

typedef struct {
	int x, y, z, pan, til;
} Player_t;

// Lighting coordinates
extern GsF_LIGHT pslt;

void CalculateCamera();
void PutObject(VECTOR pos, SVECTOR rot, GsDOBJ2 *obj);
void PutObjectScaled(VECTOR pos, SVECTOR rot, VECTOR scl, GsDOBJ2 *obj);
int LinkModel(u_long *tmd, GsDOBJ2 *obj);
void Init_3DLib();
