#include "simple_logger.h"

#include "camera.h"


Camera camera;
//get the position of the camera in World Space   mmmmmmm where World Space is my 0,0 left corner all the way to where-ever it ends
GFC_Vector2D camera_get_position() {
	return gfc_vector2d(camera.camera.x, camera.camera.y);
}


GFC_Vector2D camera_get_offset() { //I acc don't understand why the offset is negative.   Are we SUPPOSED to subtract the offset??  Can't we just add the position?
	return gfc_vector2d(-camera.camera.x, -camera.camera.y);
}

//Given a point in Worldspace.   ex:  x = 1k  y = 800.   CHANGE my camera's position (which is also in World space) to BE that position. just... centered, ofc
void camera_center_on(GFC_Vector2D point) {
	camera.camera.x = point.x - (camera.camera.w * 0.5);
	camera.camera.y = point.y - (camera.camera.h * 0.5);
}

void camera_bounds_check() {
	if (camera.camera.x < camera.bounds.x) camera.camera.x = camera.bounds.x;
	if (camera.camera.y < camera.bounds.y) camera.camera.y = camera.bounds.y;

	if (camera.camera.x + camera.camera.w > camera.bounds.x + camera.bounds.w) camera.camera.x = camera.bounds.x + camera.bounds.w - camera.camera.w; 
	if (camera.camera.y + camera.camera.h > camera.bounds.y + camera.bounds.h) camera.camera.y = camera.bounds.y + camera.bounds.h - camera.camera.h;
	

}

void camera_set_position(GFC_Vector2D position){
	gfc_vector2d_copy(camera.camera, position);
}

void camera_set_size(GFC_Vector2D size) {
	camera.camera.w = size.x;
	camera.camera.h = size.y;
}

void camera_set_bounds(GFC_Rect bounds) {
	gfc_rect_copy(camera.bounds, bounds);
	//gfc_rect_slog(camera.bounds);
}

GFC_Rect camera_get_bounds() {
	return (camera.bounds);
}
