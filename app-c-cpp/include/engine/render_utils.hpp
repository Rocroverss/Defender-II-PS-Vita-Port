#pragma once

#include <vitaGL.h>

namespace defender {

inline void draw_quad(float x, float y, float w, float h, float r, float g, float b, float a) {
    const GLfloat vertices[] = {
        x, y, 0.0f,
        x + w, y, 0.0f,
        x, y + h, 0.0f,
        x + w, y + h, 0.0f
    };

    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

inline void draw_textured_quad(GLuint texture, float x, float y, float w, float h, float alpha = 1.0f) {
    if (texture == 0) {
        return;
    }

    const GLfloat vertices[] = {
        x, y, 0.0f,
        x + w, y, 0.0f,
        x, y + h, 0.0f,
        x + w, y + h, 0.0f
    };

    const GLfloat uvs[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

inline void draw_textured_quad_uv(
    GLuint texture,
    float x,
    float y,
    float w,
    float h,
    float u0,
    float v0,
    float u1,
    float v1,
    float alpha = 1.0f
) {
    if (texture == 0) {
        return;
    }

    const GLfloat vertices[] = {
        x, y, 0.0f,
        x + w, y, 0.0f,
        x, y + h, 0.0f,
        x + w, y + h, 0.0f
    };

    const GLfloat uvs[] = {
        u0, v0,
        u1, v0,
        u0, v1,
        u1, v1
    };

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

inline void draw_textured_quad_tinted(
    GLuint texture,
    float x,
    float y,
    float w,
    float h,
    float r,
    float g,
    float b,
    float alpha = 1.0f
) {
    if (texture == 0) {
        return;
    }

    const GLfloat vertices[] = {
        x, y, 0.0f,
        x + w, y, 0.0f,
        x, y + h, 0.0f,
        x + w, y + h, 0.0f
    };

    const GLfloat uvs[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(r, g, b, alpha);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

inline void draw_textured_quad_cover(
    GLuint texture,
    float dst_x,
    float dst_y,
    float dst_w,
    float dst_h,
    float src_w,
    float src_h,
    float alpha = 1.0f
) {
    if (texture == 0 || dst_w <= 0.0f || dst_h <= 0.0f || src_w <= 0.0f || src_h <= 0.0f) {
        return;
    }

    const float src_aspect = src_w / src_h;
    const float dst_aspect = dst_w / dst_h;

    float crop_u0 = 0.0f;
    float crop_u1 = 1.0f;
    float crop_top = 0.0f;
    float crop_bottom = 1.0f;

    if (src_aspect > dst_aspect) {
        const float keep_w = src_h * dst_aspect;
        const float crop_x = (src_w - keep_w) * 0.5f;
        crop_u0 = crop_x / src_w;
        crop_u1 = (crop_x + keep_w) / src_w;
    } else if (src_aspect < dst_aspect) {
        const float keep_h = src_w / dst_aspect;
        const float crop_y = (src_h - keep_h) * 0.5f;
        crop_top = crop_y / src_h;
        crop_bottom = (crop_y + keep_h) / src_h;
    }

    draw_textured_quad_uv(
        texture,
        dst_x,
        dst_y,
        dst_w,
        dst_h,
        crop_u0,
        1.0f - crop_top,
        crop_u1,
        1.0f - crop_bottom,
        alpha
    );
}

inline void draw_textured_quad_centered(GLuint texture, float cx, float cy, float w, float h, float angle_deg, float alpha = 1.0f) {
    if (texture == 0) {
        return;
    }
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angle_deg, 0.0f, 0.0f, 1.0f);
    draw_textured_quad(texture, -w * 0.5f, -h * 0.5f, w, h, alpha);
    glPopMatrix();
}

inline void draw_textured_quad_centered_uv(
    GLuint texture,
    float cx,
    float cy,
    float w,
    float h,
    float u0,
    float v0,
    float u1,
    float v1,
    float angle_deg,
    float alpha = 1.0f
) {
    if (texture == 0) {
        return;
    }
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angle_deg, 0.0f, 0.0f, 1.0f);
    draw_textured_quad_uv(texture, -w * 0.5f, -h * 0.5f, w, h, u0, v0, u1, v1, alpha);
    glPopMatrix();
}

inline void draw_textured_quad_pivoted(
    GLuint texture,
    float pivot_x,
    float pivot_y,
    float w,
    float h,
    float pivot_offset_x,
    float pivot_offset_y,
    float angle_deg,
    float alpha = 1.0f
) {
    if (texture == 0) {
        return;
    }
    glPushMatrix();
    glTranslatef(pivot_x, pivot_y, 0.0f);
    glRotatef(angle_deg, 0.0f, 0.0f, 1.0f);
    draw_textured_quad(texture, -pivot_offset_x, -pivot_offset_y, w, h, alpha);
    glPopMatrix();
}

inline void draw_textured_quad_pivoted_uv(
    GLuint texture,
    float pivot_x,
    float pivot_y,
    float w,
    float h,
    float u0,
    float v0,
    float u1,
    float v1,
    float pivot_offset_x,
    float pivot_offset_y,
    float angle_deg,
    float alpha = 1.0f
) {
    if (texture == 0) {
        return;
    }
    glPushMatrix();
    glTranslatef(pivot_x, pivot_y, 0.0f);
    glRotatef(angle_deg, 0.0f, 0.0f, 1.0f);
    draw_textured_quad_uv(texture, -pivot_offset_x, -pivot_offset_y, w, h, u0, v0, u1, v1, alpha);
    glPopMatrix();
}

}
