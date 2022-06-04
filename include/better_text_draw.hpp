#ifndef BETTER_TEXT_DRAW_HPP
#define BETTER_TEXT_DRAW_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <exception>
#include <tuple>
#include <sstream>

namespace g80 {
    
    using namespace std::placeholders;
    using expression_map = std::map<std::string, std::function<auto (const std::string &, int16_t &) -> void>>;

    class better_text_draw {

    public:

        // THROW if size = 0
        better_text_draw(const int16_t width, const int16_t height) : 
            width_(width), height_(height), size_(width_ * height_),
            buffer_ch_(width_ * height_, ch_),
            buffer_col_(width_ * height_, col_) {

            // TODO: Could be a template? WHY? so expressionmap can be made static per widht_ height
            expression_map_["x"] = std::bind(&better_text_draw::set_x, this, _1, _2);
            expression_map_["y"] = std::bind(&better_text_draw::set_y, this, _1, _2);
            expression_map_["ch"] = std::bind(&better_text_draw::set_ch, this, _1, _2);
            expression_map_["col"] = std::bind(&better_text_draw::set_col, this, _1, _2);
            expression_map_["l"] = std::bind(&better_text_draw::draw_left, this, _1, _2);
            expression_map_["r"] = std::bind(&better_text_draw::draw_right, this, _1, _2);
            expression_map_["u"] = std::bind(&better_text_draw::draw_up, this, _1, _2); 
            expression_map_["d"] = std::bind(&better_text_draw::draw_down, this, _1, _2);
            expression_map_["ul"] = std::bind(&better_text_draw::draw_upper_left, this, _1, _2);
            expression_map_["ur"] = std::bind(&better_text_draw::draw_upper_right, this, _1, _2);
            expression_map_["ll"] = std::bind(&better_text_draw::draw_lower_left, this, _1, _2);
            expression_map_["lr"] = std::bind(&better_text_draw::draw_lower_right, this, _1, _2);
            expression_map_["arc"] = std::bind(&better_text_draw::catch_all, this, _1, _2);
            expression_map_["fil"] = std::bind(&better_text_draw::catch_all, this, _1, _2);
            expression_map_["t"] = std::bind(&better_text_draw::catch_all, this, _1, _2);
            expression_map_["tcx"] = std::bind(&better_text_draw::catch_all, this, _1, _2);
            expression_map_["tcy"] = std::bind(&better_text_draw::catch_all, this, _1, _2);
        }

        auto eval(const std::string &command) -> bool {
            try {
                int16_t cix = 0;
                skip_spaces(command, cix);
            
                do {
                    std::string exp = get_command(command, cix);
                    auto f = expression_map_.find(command);
                    if (f == expression_map_.end()) throw std::runtime_error(std::string("Could not evaluate -> ") + command + "\n");
                    
                    (f->second)(command, cix);
                    skip_spaces(command, cix);
                } while (cix < command.size());

                return true;

            } catch (std::exception e) {
                std::cout << e.what() << std::endl;
                return false;
            }
        }

        auto show(bool clear_screen = false) const -> void {
            // todo: all size vars should be changed to size_t
            std::stringstream output;
            if (clear_screen) output << "\033[2J";
            
            int16_t next_line = width_, i = 0, current_color = -1;
            do {
                if (current_color != buffer_col_[i]) {
                    current_color = buffer_col_[i];
                    output << color_[current_color];
                }

                output << buffer_ch_[i++];
                if (i == next_line) {output << "\n"; next_line += width_;}
            } while (i != size_);

            output << "\033[0m\n";
            std::cout << output.str();
        }
    private:

        uint8_t ch_{32};
        uint8_t col_{7};
        int16_t pix_{0};
        int16_t width_, height_, size_;
        std::vector<uint8_t> buffer_ch_;
        std::vector<uint8_t> buffer_col_; 
        expression_map expression_map_;

        static const std::string color_[];
        static const size_t sizeof_color_;
            
    private:

        auto set_x(const std::string &command, int16_t &cix) -> void {
            pix_ = ix(get_num_from_command(command, cix), current_y());
        }

        auto set_y(const std::string &command, int16_t &cix) -> void {
            pix_ = ix(current_x(), get_num_from_command(command, cix));
        }

        auto set_ch(const std::string &command, int16_t &cix) -> void {
            ch_ = get_ch_from_command(command, cix);
        }

        auto set_col(const std::string &command, int16_t &cix) -> void {
            auto t = get_num_from_command(command, cix);
            col_ = t >= sizeof_color_ ? t % sizeof_color_ : t;
        }

        auto draw_right(const std::string &command, int16_t &cix) -> void {
            std::cout << "r";
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x + move, y);
        }

        auto draw_left(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x - move, y);
        }

        auto draw_up(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x, y - move);
        }

        auto draw_down(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x, y + move);
        }

        auto draw_upper_left(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x - move, y - move);
        }

        auto draw_upper_right(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x + move, y - move);
        }

        auto draw_lower_right(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x + move, y + move);
        }

        auto draw_lower_left(const std::string &command, int16_t &cix) -> void {
            auto move = get_num_from_command(command, cix);
            auto [x, y] = current_xy();
            line(x - move, y + move);
        }

        auto catch_all(const std::string &command, int16_t &cix) -> void {

        }

    /**
     * Private static helpers 
     * for public interface 
     * 
     */

    private: 

        static auto throw_if_end_is_reached(const std::string &command, const int16_t &cix) -> void {
            if (cix == command.size()) throw std::runtime_error(std::string("End is reached at i:") + std::to_string(cix));
        }

        static auto throw_if_is_not_number(const std::string &command, const int16_t &cix) -> void {
            if (is_not_number(command[cix])) throw std::runtime_error(std::string("Expecting a number at i:") + std::to_string(cix));
        }

        static auto throw_if_is_not_ch(const std::string &command, const int16_t &cix) -> void {
            if (is_not_ch(command[cix])) throw std::runtime_error(std::string("Expecting a char command at i:") + std::to_string(cix));
        }

        static auto skip_spaces(const std::string &command, int16_t &cix) -> void {
            while(cix < command.size() && command[cix] == ' ') ++cix;
        }

        static inline auto is_number(const uint8_t ch) -> bool {
            return ch >= '0' && ch <= '9';
        }

        static inline auto is_not_number(const uint8_t ch) -> bool {
            return ch < '0' || ch > '9';
        }

        static inline auto is_ch(const uint8_t ch) -> bool {
            return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
        }

        static inline auto is_not_ch(const uint8_t ch) -> bool {
            return !is_ch(ch);
        }

        static auto get_num_from_command(const std::string &command, int16_t &cix) -> int16_t {
            
            skip_spaces(command, cix);
            throw_if_end_is_reached(command, cix);
            throw_if_is_not_number(command, cix);            
            
            int16_t num = 0;
            while (cix != command.size() && is_number(command[cix])) {
                num *= 10;
                num += command[cix++] - '0';
            }

            return num;
        }

        static auto get_command(const std::string &command, int16_t &cix) -> std::string {
            
            throw_if_end_is_reached(command, cix);
            throw_if_is_not_ch(command, cix);            
            
            std::string val;
            while (cix != command.size() && is_ch(command[cix])) val += command[cix++];

            return val;
        }

        static auto get_ch_from_command(const std::string &command, int16_t &cix) -> uint8_t {
            
            skip_spaces(command, cix);
            throw_if_end_is_reached(command, cix);
            
            return command[cix++];
        }

    private:

        auto line(const int16_t x, const int16_t y) -> void {

            auto [cx, cy] = current_xy();
            int16_t dx = x - cx;
            int16_t dy = y - cy;
            int16_t sdx = dx < 0 ? -1 : 1;
            int16_t sdy = dy < 0 ? -width_ : width_;
            int16_t adx = dx < 0 ? dx * -1 : dx;
            int16_t ady = dy < 0 ? dy * -1 : dy;
            
            if (adx >= ady) {    
                for (int16_t i = 0, t = ady; i <= adx; ++i, t += ady) {
                    update_ch();
                    update_col();

                    if (t >= adx) {
                        pix_ += sdy;
                        t -= adx;
                    }
                    pix_ +=sdx;
                }
            } else {
                for (int16_t i = 0, t = adx; i <= ady; ++i, t += adx) {
                    update_ch();
                    update_col();
                    
                    if (t >= ady) {
                        pix_ += sdx;
                        t -= ady;
                    }
                    pix_ += sdy;
                }
            }
        }    

    private:


        inline auto current_x() const -> int16_t {
            return pix_ % size_;
        }
        
        inline auto current_y() const -> int16_t {
            return pix_ / size_;
        }

        inline auto current_xy() const -> std::tuple<int16_t, int16_t> {
            return {current_x(), current_y()};
        }

        inline auto ix(const int16_t x, const int16_t y) const -> const int16_t {
            auto t = y * width_ + x;
            return t >= size_ ? t % size_ : t;
        }

        auto update_ch() -> void {
            if (pix_ >= size_) pix_ %= size_;
            buffer_ch_[pix_] = ch_;
        }
        
        auto update_col() -> void {
            if (pix_ >= size_) pix_ %= size_;
            buffer_col_[pix_] = col_;
        }
    };

    const std::string better_text_draw::color_[] { "\033[30m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m" };
    const size_t better_text_draw::sizeof_color_{sizeof(better_text_draw::color_) / sizeof(std::string)};
}
#endif 