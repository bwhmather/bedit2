/* Copyright 2025 Ben Mather <bwhmather@bwhmather.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
namespace Bedit {

// "Borrowed" from Gnome Text Editor and ported line by line to vala.
// Gnome Text Editor appears to have lifted the same function from Gedit.
internal string
font_description_to_css(Pango.FontDescription font_desc) {
    var builder = new StringBuilder();
    var mask = font_desc.get_set_fields();

    if ((mask & Pango.FontMask.FAMILY) != 0) {
        builder.append("font-family: \"%s\";".printf(font_desc.get_family()));
    }

    if ((mask & Pango.FontMask.VARIANT) != 0) {
        switch (font_desc.get_variant()) {
            case Pango.Variant.NORMAL:
                builder.append("font-variant: normal;");
                break;
            case Pango.Variant.SMALL_CAPS:
                builder.append("font-variant: small-caps;");
                break;
            case Pango.Variant.ALL_SMALL_CAPS:
                builder.append("font-variant: all-small-caps;");
                break;
            case Pango.Variant.PETITE_CAPS:
                builder.append("font-variant: petite-caps;");
                break;
            case Pango.Variant.ALL_PETITE_CAPS:
                builder.append("font-variant: all-petite-caps;");
                break;
            case Pango.Variant.UNICASE:
                builder.append("font-variant: unicase;");
                break;
            case Pango.Variant.TITLE_CAPS:
                builder.append("font-variant: titling-caps;");
                break;
            default:
                break;
        }
    }

    if ((mask & Pango.FontMask.WEIGHT) != 0) {
        int weight = font_desc.get_weight();

        // WORKAROUND:
        //
        // font-weight with numbers does not appear to be working as expected
        // right now. So for the common (bold/normal), let's just use the string
        // and let gtk warn for the other values, which shouldn't really be
        // used for this.

        switch (weight) {
            case Pango.Weight.SEMILIGHT:
                // 350 is not actually a valid css font-weight, so we will just round
                // up to 400.
            case Pango.Weight.NORMAL:
                builder.append("font-weight: normal;");
                break;
            case Pango.Weight.BOLD:
                builder.append("font-weight: bold;");
                break;
            case Pango.Weight.THIN:
            case Pango.Weight.ULTRALIGHT:
            case Pango.Weight.LIGHT:
            case Pango.Weight.BOOK:
            case Pango.Weight.MEDIUM:
            case Pango.Weight.SEMIBOLD:
            case Pango.Weight.ULTRABOLD:
            case Pango.Weight.HEAVY:
            case Pango.Weight.ULTRAHEAVY:
            default:
                // round to nearest hundred
                weight = (int) Math.round(weight / 100.0) * 100;
                builder.append("font-weight: %d;".printf(weight));
                break;
        }
    }

    if ((mask & Pango.FontMask.STRETCH) != 0) {
        switch (font_desc.get_stretch()) {
            case Pango.Stretch.ULTRA_CONDENSED:
                builder.append("font-stretch: ultra-condensed;");
                break;
            case Pango.Stretch.EXTRA_CONDENSED:
                builder.append("font-stretch: extra-condensed;");
                break;
            case Pango.Stretch.CONDENSED:
                builder.append("font-stretch: condensed;");
                break;
            case Pango.Stretch.SEMI_CONDENSED:
                builder.append("font-stretch: semi-condensed;");
                break;
            case Pango.Stretch.NORMAL:
                builder.append("font-stretch: normal;");
                break;
            case Pango.Stretch.SEMI_EXPANDED:
                builder.append("font-stretch: semi-expanded;");
                break;
            case Pango.Stretch.EXPANDED:
                builder.append("font-stretch: expanded;");
                break;
            case Pango.Stretch.EXTRA_EXPANDED:
                builder.append("font-stretch: extra-expanded;");
                break;
            case Pango.Stretch.ULTRA_EXPANDED:
                builder.append("font-stretch: ultra-expanded;");
                break;
            default:
                break;
        }
    }

    if ((mask & Pango.FontMask.SIZE) != 0) {
        int font_size = font_desc.get_size() / Pango.SCALE;
        builder.append("font-size: %dpt;".printf(font_size));
    }

    return builder.str;
}

}
