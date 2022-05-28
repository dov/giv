//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
// Gunnar Roth: add support for linear and radial gradients(support xlink attr),
// shape gradient opaqueness, rounded rects, circles,ellipses. support a command (arc)  in pathes. 
// set new origin correctly to last postion on z command in a path( was set to 0,0 before).
// enable parsing of colors written as rgb()
// some code was inspired by code from Haiku OS
/*
* Copyright 2006-2007, Haiku. All rights reserved.
* Distributed under the terms of the MIT License.
*
* Authors:
* Stephan Aßmus <superstippi@gmx.de>
*/
//----------------------------------------------------------------------------
//
//
// SVG path renderer.
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include "agg_svg_path_renderer.h"

namespace agg
{
namespace svg
{

    //------------------------------------------------------------------------
    path_renderer::path_renderer() :
        m_cur_gradient(NULL),
        m_curved(m_storage),
        m_curved_count(m_curved),

        m_curved_stroked(m_curved_count),
        m_curved_stroked_trans(m_curved_stroked, m_transform),

        m_curved_trans(m_curved_count, m_transform),
        m_curved_trans_contour(m_curved_trans),
        m_paint_by_label(false)
    {
        m_curved_trans_contour.auto_detect_orientation(false);
    }


    //------------------------------------------------------------------------
    void path_renderer::remove_all()
    {
        m_storage.remove_all();
        m_attr_storage.remove_all();
        m_attr_stack.remove_all();
        m_transform.reset();
    }

    //------------------------------------------------------------------------
    void path_renderer::begin_path()
    {
        push_attr();
        unsigned idx = m_storage.start_new_path();
        m_attr_storage.add(path_attributes(cur_attr(), idx));
    }

    //------------------------------------------------------------------------
    void path_renderer::end_path()
    {
        if(m_attr_storage.size() == 0) 
        {
            throw exception("end_path : The path was not begun");
        }
        path_attributes attr = cur_attr();
        unsigned idx = m_attr_storage[m_attr_storage.size() - 1].index;
        attr.index = idx;
        m_attr_storage[m_attr_storage.size() - 1] = attr;
        pop_attr();
    }

    //------------------------------------------------------------------------
    void path_renderer::move_to(double x, double y, bool rel)          // M, m
    {
        if(rel && m_storage.total_vertices()) {
           double x2,y2;
           unsigned cmd = m_storage.last_vertex(&x2,&y2);
           if(is_end_poly(cmd)) {
               // rewind until we find a real vertex
               unsigned idx = m_storage.total_vertices()-1;
               while(idx && !is_vertex(m_storage.vertex(--idx,&x2,&y2)));
           }
           x += x2;
           y += y2;
        }
        m_storage.move_to(x, y);
    }

    //-------------------------------------------------------
    void path_renderer::set_balloon(const char *balloon)
    {
        path_attributes& attr = cur_attr();
        m_balloon_labels.push_back(balloon);
        attr.set_balloon_index(m_balloon_labels.size()-1);
    }
  
    //------------------------------------------------------------------------
    void path_renderer::line_to(double x,  double y, bool rel)         // L, l
    {
        if(rel) m_storage.rel_to_abs(&x, &y);
        m_storage.line_to(x, y);
    }

    //------------------------------------------------------------------------
    void path_renderer::hline_to(double x, bool rel)                   // H, h
    {
        double x2 = 0.0;
        double y2 = 0.0;
        if(m_storage.total_vertices())
        {
            m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
            if(rel) x += x2;
            m_storage.line_to(x, y2);
        }
    }

    //------------------------------------------------------------------------
    void path_renderer::vline_to(double y, bool rel)                   // V, v
    {
        double x2 = 0.0;
        double y2 = 0.0;
        if(m_storage.total_vertices())
        {
            m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
            if(rel) y += y2;
            m_storage.line_to(x2, y);
        }
    }

    //------------------------------------------------------------------------
    void path_renderer::curve3(double x1, double y1,                   // Q, q
                               double x,  double y, bool rel)
    {
        if(rel) 
        {
            m_storage.rel_to_abs(&x1, &y1);
            m_storage.rel_to_abs(&x,  &y);
        }
        m_storage.curve3(x1, y1, x, y);
    }

    //------------------------------------------------------------------------
    void path_renderer::curve3(double x, double y, bool rel)           // T, t
    {
//        throw exception("curve3(x, y) : NOT IMPLEMENTED YET");
        if(rel) 
        {
            m_storage.curve3_rel(x, y);
        } else 
        {
            m_storage.curve3(x, y);
        }
    }

    //------------------------------------------------------------------------
    void path_renderer::curve4(double x1, double y1,                   // C, c
                               double x2, double y2, 
                               double x,  double y, bool rel)
    {
        if(rel) 
        {
            m_storage.rel_to_abs(&x1, &y1);
            m_storage.rel_to_abs(&x2, &y2);
            m_storage.rel_to_abs(&x,  &y);
        }
        m_storage.curve4(x1, y1, x2, y2, x, y);
    }

    //------------------------------------------------------------------------
    void path_renderer::curve4(double x2, double y2,                   // S, s
                               double x,  double y, bool rel)
    {
        //throw exception("curve4(x2, y2, x, y) : NOT IMPLEMENTED YET");
        if(rel) 
        {
            m_storage.curve4_rel(x2, y2, x, y);
        } else 
        {
            m_storage.curve4(x2, y2, x, y);
        }
    }

    void path_renderer::arc_to(double rx, double ry,                   // A,a
                               double xrot,
                               bool large_arc_flag,
                               bool sweep_flag,
                               double x,
                               double y,
                               bool rel
                               )
    {
        if(rel) m_storage.rel_to_abs(&x, &y);
        m_storage.arc_to(rx,ry,xrot,large_arc_flag,sweep_flag,x,y);
    }

    // elliptical_arc
    void
    path_renderer::elliptical_arc(double rx, double ry, double angle,
                                  bool large_arc_flag, bool sweep_flag,
                                  double x, double y, bool rel)
    {
        angle = angle / 180.0 * pi;
        if (rel) {
            m_storage.arc_rel(rx, ry, angle, large_arc_flag, sweep_flag, x, y);
        } else {
            m_storage.arc_to(rx, ry, angle, large_arc_flag, sweep_flag, x, y);
        }
    }

    //------------------------------------------------------------------------
    void path_renderer::close_subpath()
    {
        m_storage.end_poly(path_flags_close);
    }

    //------------------------------------------------------------------------
    path_attributes& path_renderer::cur_attr()
    {
        if(m_attr_stack.size() == 0)
        {
            throw exception("cur_attr : Attribute stack is empty");
        }
        return m_attr_stack[m_attr_stack.size() - 1];
    }

    //------------------------------------------------------------------------
    void path_renderer::push_attr()
    {
        m_attr_stack.add(m_attr_stack.size() ? 
                         m_attr_stack[m_attr_stack.size() - 1] :
                         path_attributes());
    }

    //------------------------------------------------------------------------
    void path_renderer::pop_attr()
    {
        if(m_attr_stack.size() == 0)
        {
            throw exception("pop_attr : Attribute stack is empty");
        }
        m_attr_stack.remove_last();
    }

    //------------------------------------------------------------------------
    void path_renderer::fill(const rgba8& f)
    {
        path_attributes& attr = cur_attr();
        if (m_paint_by_label) {
            if (attr.label_index>=0)
                attr.fill_color = attr.get_label_color(this->m_balloon_base_index);
            else
                attr.fill_color = m_label_color;
        }
        else
            attr.fill_color = f;
        
        attr.fill_flag = true;
    }

    //------------------------------------------------------------------------
    void path_renderer::stroke(const rgba8& s)
    {
        path_attributes& attr = cur_attr();
        if (m_paint_by_label) {
            if (attr.label_index>=0)
                attr.fill_color = attr.get_label_color(this->m_balloon_base_index);
            else
                attr.fill_color = m_label_color;
        }
        else
          attr.stroke_color = s;
        attr.stroke_flag = true;
    }

    //------------------------------------------------------------------------
    void path_renderer::even_odd(bool flag)
    {
        cur_attr().even_odd_flag = flag;
    }
    
    //------------------------------------------------------------------------
    void path_renderer::stroke_width(double w)
    {
        cur_attr().stroke_width = w;
    }

    //------------------------------------------------------------------------
    void path_renderer::fill_none()
    {
        cur_attr().fill_flag = false;
    }

    // fill_url
    void path_renderer::fill_url(const char* url)
    {
        sprintf(cur_attr().fill_url, "%s", url);
    }

    //------------------------------------------------------------------------
    void path_renderer::stroke_none()
    {
        cur_attr().stroke_flag = false;
    }

    // stroke_url
    void
    path_renderer::stroke_url(const char* url)
    {
        sprintf(cur_attr().stroke_url, "%s", url);
    }
    
    // opacity
    void
    path_renderer::opacity(double op)
    {
        cur_attr().opacity *= op;
    }
    
    //------------------------------------------------------------------------
    void path_renderer::fill_opacity(double op)
    {
        if (!m_paint_by_label)
            cur_attr().fill_color.opacity(op);
    }
    
    //------------------------------------------------------------------------
    void path_renderer::stroke_opacity(double op)
    {
        if (!m_paint_by_label)
            cur_attr().stroke_color.opacity(op);
    }

    //------------------------------------------------------------------------
    void path_renderer::line_join(line_join_e join)
    {
        cur_attr().line_join = join;
    }

    //------------------------------------------------------------------------
    void path_renderer::line_cap(line_cap_e cap)
    {
        cur_attr().line_cap = cap;
    }

    //------------------------------------------------------------------------
    void path_renderer::miter_limit(double ml)
    {
        cur_attr().miter_limit = ml;
    }

    //------------------------------------------------------------------------
    trans_affine& path_renderer::transform()
    {
        return cur_attr().transform;
    }

    //------------------------------------------------------------------------
    void path_renderer::parse_path(path_tokenizer& tok)
    {
        char lastCmd = 0;
        while(tok.next())
        {
            double arg[10];
            char cmd = tok.last_command();
            if (cmd == 0)
                cmd = 'l';

            unsigned i;
            switch(cmd)
            {
                case 'M': case 'm':
                    arg[0] = tok.last_number();
                    arg[1] = tok.next(cmd);
                    if (lastCmd != cmd)
                      move_to(arg[0], arg[1], cmd == 'm');
                    else
                      line_to(arg[0], arg[1], lastCmd == 'm');

                    // An m command is implicitely followed by l commands
                    if (cmd=='m')
                        tok.set_last_command('l');
                    else
                        tok.set_last_command('L');
                    break;

                case 'L': case 'l':
                    arg[0] = tok.last_number();
                    arg[1] = tok.next(cmd);
                    line_to(arg[0], arg[1], cmd == 'l');
                    break;

                case 'V': case 'v':
                    vline_to(tok.last_number(), cmd == 'v');
                    break;

                case 'H': case 'h':
                    hline_to(tok.last_number(), cmd == 'h');
                    break;
                
                case 'Q': case 'q':
                    arg[0] = tok.last_number();
                    for(i = 1; i < 4; i++)
                    {
                        arg[i] = tok.next(cmd);
                    }
                    curve3(arg[0], arg[1], arg[2], arg[3], cmd == 'q');
                    break;

                case 'T': case 't':
                    arg[0] = tok.last_number();
                    arg[1] = tok.next(cmd);
                    curve3(arg[0], arg[1], cmd == 't');
                    break;

                case 'C': case 'c':
                    arg[0] = tok.last_number();
                    for(i = 1; i < 6; i++)
                    {
                        arg[i] = tok.next(cmd);
                    }
                    curve4(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], cmd == 'c');
                    break;

                case 'S': case 's':
                    arg[0] = tok.last_number();
                    for(i = 1; i < 4; i++)
                    {
                        arg[i] = tok.next(cmd);
                    }
                    curve4(arg[0], arg[1], arg[2], arg[3], cmd == 's');
                    break;

                case 'A': case 'a': {
                    arg[0] = tok.last_number();
                    for(i = 1; i < 3; i++) {
                      arg[i] = tok.next(cmd);
                    }
                    bool large_arc_flag = tok.next(cmd) ? true : false;
                    bool sweep_flag = tok.next(cmd) ? true : false;
                    for(i = 3; i < 5; i++) {
                       arg[i] = tok.next(cmd);
                    }
                    elliptical_arc(arg[0], arg[1], arg[2],
                                    large_arc_flag, sweep_flag,
                                    arg[3], arg[4], cmd == 'a');
                    break;
                    }
                case 'Z': case 'z':
                    {
                        double x = m_storage.last_x();
                        double y = m_storage.last_y();
                        close_subpath();
                        move_to(x,y,false);
                    }
                    break;

                default:
                {
                    char buf[100];
                    sprintf(buf, "parse_path: Invalid Command %c", cmd);
                    throw exception(buf);
                }
            }
            lastCmd = cmd;
        }
    }
    
    void path_renderer::start_gradient(bool radial)
    {
        if (m_cur_gradient) {
            fprintf(stderr, "path_renderer::StartGradient() - ERROR: "
                "previous gradient (%s) not finished!\n",
                m_cur_gradient->id());
        }

        if (radial)
            m_cur_gradient = new radial_gradient();
        else
            m_cur_gradient = new linear_gradient();

        add_gradient(m_cur_gradient);
    }


    void path_renderer::end_gradient()
    {
        if (m_cur_gradient) {
            m_cur_gradient->realize();
        } else {
            fprintf(stderr, "path_renderer::EndGradient() - "
                "ERROR: no gradient started!\n");
        }
        m_cur_gradient = NULL;
    }

    // #pragma mark -

    // _AddGradient
    void
    path_renderer::add_gradient(gradient* gradient)
    {
        if (gradient) {
            m_gradients.push_back(gradient);
        }
    }

    // _GradientAt
    gradient*
    path_renderer::gradient_at(int32 index) const
    {
        return m_gradients.at(index);
    }

    // _FindGradient
    gradient*
    path_renderer::find_gradient(const char* name) const
    {
        for (int32 i = 0; i < (int)m_gradients.size(); i++) {
            gradient* g = gradient_at(i);
            if (strcmp(g->id(), name) == 0)
                return g;
        }
        return NULL;
    }
}
}

