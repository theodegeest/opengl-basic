#include "shape.h"
#include "quad.h"

ShapeBox shape_box_create(float x, float y, float width, float height,
                          float thickness, Color color, float texture_id) {
  Quad left =
      quad_create(x, y, 0.0f, thickness, height, 0.0f, color, texture_id);
  Quad up = quad_create(x, y + height - thickness, 0.0f, width, thickness, 0.0f,
                        color, texture_id);
  Quad right = quad_create(x + width - thickness, y, 0.0f, thickness, height,
                           0.0f, color, texture_id);
  Quad down =
      quad_create(x, y, 0.0f, width, thickness, 0.0f, color, texture_id);

  return (ShapeBox){left, up, right, down};
}

void shape_box_color_set(ShapeBox *sb, Color color) {
  quad_color_set(&sb->left, color);
  quad_color_set(&sb->up, color);
  quad_color_set(&sb->right, color);
  quad_color_set(&sb->down, color);
}

void shape_box_move(ShapeBox *sb, float x, float y, float width, float height) {

  float thickness = sb->left.rd.pos.x - sb->left.ld.pos.x;
  quad_move(&sb->left, x, y, 0.0f, thickness, height, 0.0f);
  quad_move(&sb->up, x, y + height - thickness, 0.0f, width, thickness, 0.0f);
  quad_move(&sb->right, x + width - thickness, y, 0.0f, thickness, height,
            0.0f);
  quad_move(&sb->down, x, y, 0.0f, width, thickness, 0.0f);
}
