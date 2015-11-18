#pragma once

class Cards
{
	public:
		void setProperties(int h, int m, int d) {
			hp = h;
			mana = m;
			damage = d;
		}

		int getHp() {
			return hp;
		}
	private:
		int hp;
		int mana;
		int damage;
};