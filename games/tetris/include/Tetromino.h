#ifndef TETROMINO_H
#define TETROMINO_H

#include <array>
#include <cxxg/Types.h>
#include <string>
#include <vector>

namespace cxxg {
class Screen;
}

class Tetromino {
public:
  enum class Rotation {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3,
  };

  static constexpr std::array Rotations = {
      Rotation::Up,
      Rotation::Right,
      Rotation::Down,
      Rotation::Left,
  };

  static const std::vector<cxxg::types::Color> Colors;

  class Iterator {
    friend bool operator==(const Iterator &Lhs, const Iterator &Rhs) noexcept;
    friend bool operator!=(const Iterator &Lhs, const Iterator &Rhs) noexcept;

  public:
    Iterator() = default;
    Iterator(const Tetromino &T) : T(&T) {
      if (getChar() == '.') {
        ++*this;
      }
    }

    Iterator &operator++() {
      Pos.X += 1;
      if (Pos.X >= 4) {
        Pos.X = 0;
        Pos.Y += 1;
      }
      if (Pos.Y >= 4) {
        T = nullptr;
        return *this;
      }
      if (getChar() == '.') {
        return ++*this;
      }
      return *this;
    }

    Iterator operator++(int) {
      auto Copy = *this;
      ++*this;
      return Copy;
    }

    std::pair<cxxg::types::Position, char> operator*() const {
      return {Pos + T->getPosition(), getChar()};
    }

  private:
    char getChar() const {
      auto RotPos = Tetromino::rotate(Pos, T->getRotation());
      return T->getShape().at(RotPos.Y).at(RotPos.X);
    }

  private:
    cxxg::types::Position Pos = {0, 0};
    const Tetromino *T = nullptr;
  };

  static const unsigned NumShapes;

public:
  explicit Tetromino(cxxg::types::Color Color,
                     std::vector<std::string> const &Shape);
  virtual ~Tetromino() = default;

  Tetromino(const Tetromino &) = default;
  Tetromino &operator=(const Tetromino &) = default;

  /// Draw tetromino to the screen at its position
  void draw(cxxg::Screen &Scr, cxxg::types::Position Pos = {0, 0});

  Iterator begin() const { return Iterator(*this); }
  Iterator end() const { return Iterator(); }

  void rotate(int Times);

  cxxg::types::Color getColor() const;

  void setRotation(Rotation Rot);
  Rotation getRotation() const;

  void setPosition(cxxg::types::Position Pos);
  cxxg::types::Position getPosition() const;

  inline Tetromino &operator+=(cxxg::types::Position Delta) {
    Pos += Delta;
    return *this;
  }
  inline Tetromino &operator-=(cxxg::types::Position Delta) {
    Pos -= Delta;
    return *this;
  }

  const std::vector<std::string> &getShape() const;

public:
  static Tetromino create(cxxg::types::Color Color, unsigned ShapeIdx);

  static cxxg::types::Position rotate(cxxg::types::Position Pos, Rotation Rot);

protected:
  /// Color of the tetromino
  cxxg::types::Color Color = cxxg::types::Color::NONE;

  /// Rotation of the tetromino in degrees
  Rotation Rot = Rotation::Up;

  /// The shape of the tetromino
  const std::vector<std::string> *Shape = nullptr;

  cxxg::types::Position Pos = {0, 0};
};

inline bool operator==(const Tetromino::Iterator &Lhs,
                       const Tetromino::Iterator &Rhs) noexcept {
  return (Lhs.T == Rhs.T && Lhs.Pos == Rhs.Pos) || (!Lhs.T && !Rhs.T);
}

inline bool operator!=(const Tetromino::Iterator &Lhs,
                       const Tetromino::Iterator &Rhs) noexcept {
  return !(Lhs == Rhs);
}

#endif // #ifndef TETROMINO_H