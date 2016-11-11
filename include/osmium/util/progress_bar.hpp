#ifndef OSMIUM_UTIL_PROGRESS_BAR_HPP
#define OSMIUM_UTIL_PROGRESS_BAR_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2016 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <iostream>

namespace osmium {

    /**
     * Displays a progress bar on STDERR. Can be used together with the
     * osmium::io::Reader class for instance.
     */
    class ProgressBar {

        static const char* bar() noexcept {
            return "======================================================================";
        }

        static const char* spc() noexcept {
            return "                                                                     ";
        }

        static constexpr const size_t length = 70;

        // The max size is the file size if there is a single file and the
        // sum of all file sizes if there are multiple files. It corresponds
        // to 100%.
        size_t m_max_size;

        // The sum of the file sizes already done.
        size_t m_done_size = 0;

        // The currently read size in the current file.
        size_t m_current_size = 0;

        // The percentage calculated when it was last displayed. Used to decide
        // whether we need to update the display. Start setting is one that
        // will always be different from any legal setting.
        size_t m_prev_percent = 100 + 1;

        // Is the progress bar enabled at all?
        bool m_enable;

        // Used to make sure we do cleanup in the destructor if it was not
        // already done.
        bool m_do_cleanup = true;

        void display() {
            const size_t percent = 100 * (m_done_size + m_current_size) / m_max_size;
            if (m_prev_percent == percent) {
                return;
            }
            m_prev_percent = percent;

            const size_t num = size_t(percent * (length / 100.0));
            std::cerr << '[';
            if (num >= length) {
                std::cerr << bar();
            } else {
                std::cerr << (bar() + length - num) << '>' << (spc() + num);
            }
            std::cerr << "] ";
            if (percent < 10) {
                std::cerr << ' ';
            }
            if (percent < 100) {
                std::cerr << ' ';
            }
            std::cerr << percent << "% \r";
        }

    public:

        /**
         * Initializes the progress bar. No output yet.
         *
         * @param max_size Max size equivalent to 100%.
         * @param enable Set to false to disable (for instance if stderr is
         *               not a TTY).
         */
        ProgressBar(size_t max_size, bool enable) noexcept :
            m_max_size(max_size),
            m_enable(max_size > 0 && enable) {
        }

        ~ProgressBar() {
            if (m_do_cleanup) {
                try {
                    done();
                } catch (...) {
                    // Swallow any exceptions, because a destructor should
                    // not throw.
                }
            }
        }

        /**
         * Call this function to update the progress bar. Actual update will
         * only happen if the percentage changed from the last time this
         * function was called.
         *
         * @param current_size Current size. Used together with the max_size
         *                     from constructor to calculate the percentage.
         */
        void update(size_t current_size) {
            if (!m_enable) {
                return;
            }

            m_current_size = current_size;

            display();
        }

        /**
         * If you are reading multiple files, call this function after each
         * file is finished.
         *
         * @param file_size The size of the file just finished.
         */
        void file_done(size_t file_size) {
            if (m_enable) {
                m_done_size += file_size;
                m_current_size = 0;
                display();
            }
        }

        /**
         * Call this at the end. Will update the progress bar to 100% and
         * print a final line feed. If this is not called explicitly the
         * destructor will also call this.
         */
        void done() {
            m_do_cleanup = false;
            if (m_enable) {
                m_done_size = m_max_size;
                m_current_size = 0;
                display();
                std::cerr << '\n';
            }
        }

        /**
         * Removes the progress bar. Call this before doing any other output.
         * The next time update() is called, the progress bar will be visible
         * again.
         */
        void remove() {
            if (m_enable) {
                std::cerr << spc() << "         \r";
                m_prev_percent = 100 + 1;
            }
        }

    }; // class ProgressBar

} // namespace osmium

#endif // OSMIUM_UTIL_PROGRESS_BAR_HPP
