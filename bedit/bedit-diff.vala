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

public delegate void Bedit.LineDiffEditFunc(int old_start, int old_count, int new_start, int new_count);

private static int count_lines(uint8[] buf) {
    int n = 0;
    foreach (var byte in buf) {
        if (byte == '\n') {
            n++;
        }
    }
    if (buf.length > 0 && buf[buf.length - 1] != '\n') {
        n++;
    }
    return n;
}

namespace Bedit {
public void line_diff(uint8[] a, uint8[] b, Bedit.LineDiffEditFunc func) {
    Ggit.init();

    var options = new Ggit.DiffOptions();
    options.n_context_lines = 0;

    try {
        var diff = new Ggit.Diff.buffers(a, null, b, null, options);
        diff.@foreach(null, null, (delta, hunk) => {
            var old_count = hunk.get_old_lines();
            var new_count = hunk.get_new_lines();
            var old_start = (old_count > 0) ? hunk.get_old_start() - 1 : hunk.get_old_start();
            var new_start = (new_count > 0) ? hunk.get_new_start() - 1 : hunk.get_new_start();
            func(old_start, old_count, new_start, new_count);
            return 0;
        }, null);
    } catch (Error err) {
        func(0, count_lines(a), 0, count_lines(b));
    }
}
}
