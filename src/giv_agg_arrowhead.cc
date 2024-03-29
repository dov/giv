//======================================================================
//  giv_agg_arrowhead.cc - Symmetric arrow heads for giv
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue May  6 23:22:47 2008
//----------------------------------------------------------------------

//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
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
//
// Simple arrowhead/arrowtail generator 
//
//----------------------------------------------------------------------------

#include "giv_agg_arrowhead.h"

namespace givagg
{
    using namespace agg;
    
    //------------------------------------------------------------------------
    arrowhead::arrowhead() :
        m_d1(1.0),
        m_d2(1.0),
        m_d3(1.0),
        m_d4(0.0),
        m_d5(0.0),
        m_head_flag(false),
        m_tail_flag(false),
        m_curr_id(0),
        m_curr_coord(0)
    {
    }



    //------------------------------------------------------------------------
    void arrowhead::rewind(unsigned path_id)
    {
        m_curr_id = path_id;
        m_curr_coord = 0;
        if(path_id == 0)
        {
            if(!m_tail_flag)
            {
                m_cmd[0] = path_cmd_stop;
                return;
            }
            m_coord[0]  = -m_d1;         m_coord[1]  = -m_d5;
            m_coord[2]  = (m_d2 + m_d4); m_coord[3]  = -m_d3;
            m_coord[4]  = m_d2;          m_coord[5]  = 0.0;
            m_coord[6]  = (m_d2 + m_d4); m_coord[7]  = m_d3;
            m_coord[8]  = -m_d1;         m_coord[9]  = m_d5;

            m_cmd[0] = path_cmd_move_to;
            m_cmd[1] = path_cmd_line_to;
            m_cmd[2] = path_cmd_line_to;
            m_cmd[3] = path_cmd_line_to;
            m_cmd[4] = path_cmd_line_to;
            m_cmd[5] = int(path_cmd_end_poly) | int(path_flags_close);
            m_cmd[6] = path_cmd_stop;

            return;
        }

        if(path_id == 1)
        {
            if(!m_head_flag)
            {
                m_cmd[0] = path_cmd_stop;
                return;
            }
            m_coord[0]  = -m_d1;         m_coord[1]  = -m_d5;
            m_coord[2]  = (m_d2 + m_d4); m_coord[3]  = -m_d3;
            m_coord[4]  = m_d2;          m_coord[5]  = 0.0;
            m_coord[6]  = (m_d2 + m_d4); m_coord[7]  = m_d3;
            m_coord[8]  = -m_d1;         m_coord[9]  = m_d5;

            m_cmd[0] = path_cmd_move_to;
            m_cmd[1] = path_cmd_line_to;
            m_cmd[2] = path_cmd_line_to;
            m_cmd[3] = path_cmd_line_to;
            m_cmd[4] = path_cmd_line_to;
            m_cmd[5] = int(path_cmd_end_poly) | int(path_flags_close) | int(path_flags_ccw);
            m_cmd[6] = path_cmd_stop;
            return;
        }
    }


    //------------------------------------------------------------------------
    unsigned arrowhead::vertex(double* x, double* y)
    {
        if(m_curr_id < 2)
        {
            unsigned curr_idx = m_curr_coord * 2;
            *x = m_coord[curr_idx];
            *y = m_coord[curr_idx + 1];
            return m_cmd[m_curr_coord++];
        }
        return path_cmd_stop;
    }

}
