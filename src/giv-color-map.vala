//======================================================================
//  This is a widget that displays the current colormap that is used.
//----------------------------------------------------------------------

using Gtk;
using Cairo;

public class GivColorMap : DrawingArea {
	public int [] colormap;
	public double min;
	public double max;
	public bool is_horizontal;

	construct {
		colormap.resize(256*3);
		this.set_default_map();

		set_size_request(80, 300);
		min = 0;
		max = 255e-5;
		is_horizontal = false;
	}

	public void set_default_map()
	{
		// Default colormap
		for (int i=0; i<256; i++) {
			colormap[i*3 + 0] = i;
			colormap[i*3 + 1] = i;
			colormap[i*3 + 2] = i;
		}
	}

	public void set_rgb_colormap(uint8 [] colormap)
	{
		if (colormap == null)
			this.set_default_map();
		else {
			for (int i=0; i<256*3; i++) 
				this.colormap[i] = colormap[i];
		}

		redraw();
	}

	public void redraw()
	{
		queue_draw_area(0,0,
						get_allocated_width(),
						get_allocated_height());
	}

	public void show_text(Cairo.Context cr,
						  string text,
						  Pango.Alignment alignment)
	{
		Pango.Rectangle ink_rect, logical_rect;
		var font_description = new Pango.FontDescription();
        font_description.set_family("Sans ");
        font_description.set_size((int)(9 * Pango.SCALE));

		var layout = Pango.cairo_create_layout(cr);
		layout.set_font_description(font_description); 
		layout.set_markup(text, -1);
		layout.get_extents(out ink_rect, out logical_rect);
		if (alignment == Pango.Alignment.CENTER) {
			cr.rel_move_to(-logical_rect.width /2.0/ Pango.SCALE,
						   -logical_rect.height/2.0/Pango.SCALE);
		}
		else if (alignment == Pango.Alignment.RIGHT) {
			cr.rel_move_to(-logical_rect.width / Pango.SCALE,
						   -logical_rect.height/2.0/Pango.SCALE);
		}
		else {
			cr.rel_move_to(0,
						   -logical_rect.height/2.0/Pango.SCALE);
		}
		Pango.cairo_show_layout(cr, layout);
	}

	public void cshow(Cairo.Context cr,
					  string text)
	{
		show_text(cr, text, Pango.Alignment.CENTER);
	}

	public void lshow(Cairo.Context cr,
					  string text)
	{
		show_text(cr, text, Pango.Alignment.LEFT);
	}

	public void rshow(Cairo.Context cr,
					  string text)
	{
		show_text(cr, text, Pango.Alignment.RIGHT);
	}

	public void set_min_max(double min,
							double max)
	{
		this.min = min;
		this.max = max;
	}

	private string double_to_markup(double v)
	{
		if (v > 100 && v < 10000) {
			return "%.0f".printf(v);
		}
		try {
			var format = "%.3g";

			var s = format.printf(v);

            // Cleanup string and turn it into 10^ notation.
			MatchInfo match_info;
			var regex = new Regex ("(.*?)e([\\+\\-])0+(\\d+)");
			if (regex.match(s, (RegexMatchFlags)0,
							out match_info)) {
				string s_sign = match_info.fetch(2);
				if (s_sign == "+") 
					s_sign = "";
					
				s = match_info.fetch(1)
					+ "&#8901;10<sup>"
					+ s_sign
					+ match_info.fetch(3)
					+ "</sup>";
			}

			// Ron - end of uncommenting 
			return s;
		} catch (RegexError e) {
		}

		return "";
	}
	
	public override bool draw(Context cr)
	{
		var w = get_allocated_width();
		var h = get_allocated_height();

		if (!is_horizontal) {
			// Draw 256 with colors from start to end
			var lmargin = 60;
			var rmargin = 5;
			var tbmargin = 10;
			var grad_span = h - 2*tbmargin;
			var line_width = 1.0*grad_span/256;
			var line_length = w - lmargin - rmargin;
			var text_pad = 3;
			cr.set_line_width (Math.ceil(line_width+0.5));
			
			for (int i=0; i<256; i++) {
				// TBD - use color table lookup
				double rr = 1.0/255*colormap[(255-i)*3+0];
				double gg = 1.0/255*colormap[(255-i)*3+1];
				double bb = 1.0/255*colormap[(255-i)*3+2];
				cr.set_source_rgb(rr, gg, bb);
				cr.move_to(lmargin            , tbmargin + i * line_width);
				cr.line_to(lmargin+line_length, tbmargin + i * line_width);
				cr.stroke();
			}
			
			// Print labels
			var label_pos_x = lmargin-text_pad;
			cr.set_source_rgb(0,0,0);
			cr.move_to(label_pos_x, tbmargin);
			rshow(cr, double_to_markup(max));
			cr.move_to(label_pos_x, tbmargin+grad_span);
			rshow(cr, double_to_markup(min));
		}
		else {
			// Draw 256 with colors from start to end
			var margin = 20;
			var grad_width = w - 2*margin;
			var line_width = 1.0*grad_width/256;
			var line_height = h - 3*margin;
			cr.set_line_width (Math.ceil(line_width+0.5));
			
			for (int i=0; i<256; i++) {
				// TBD - use color table lookup
				double rr = 1.0/255*colormap[i*3+0];
				double gg = 1.0/255*colormap[i*3+1];
				double bb = 1.0/255*colormap[i*3+2];
				cr.set_source_rgb(rr, gg, bb);
				cr.move_to(margin + i * line_width, margin);
				cr.line_to(margin + i * line_width, margin + line_height);
				cr.stroke();
			}
			
			// Print labels
			cr.set_source_rgb(0,0,0);
			cr.move_to(margin, 1.1*margin + line_height);
			cshow(cr, min.to_string());
			cr.move_to(margin+grad_width, 1.1*margin + line_height);
			cshow(cr, max.to_string());
		}

		return true;
	}
		
	
}