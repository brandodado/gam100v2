#include <math.h>
#include "utils.h"

// Checks if a specific point lies within a centered rectangle by calculating the boundaries 
// based on the center point and dimensions. Returns 1 if the click is within bounds, 0 otherwise.
int IsAreaClicked(float area_center_x, float area_center_y, float area_width, float area_height, float click_x, float click_y)
{
	float left_border = area_center_x - (area_width / 2);
	float right_border = area_center_x + (area_width / 2);
	float top_border = area_center_y - (area_height / 2);
	float bottom_border = area_center_y + (area_height / 2);
	if (left_border <= click_x && right_border >= click_x && top_border <= click_y && bottom_border >= click_y)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}