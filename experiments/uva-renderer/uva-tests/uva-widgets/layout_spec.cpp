#include <iostream>

#include <layout.hpp>

#include <uva/tests.hpp>

using namespace uva::tests;

describe of(structure, "layout", []() {
    it("should be flexible by default", [](){
        layout l;
        expect(l.style.type).to<eq>(layout_style::layout_type::flexible);
    });

    describe("childreans", [](){
        it("should be empty by default", [](){
            layout l;
            expect(l.childreans.size()).to<eq>(0);
        });
    });

    describe("flexible", [](){
        it("should have a vertical direction by default", [](){
            layout l;
            expect(l.style.direction).to<eq>(layout_style::layout_flex_direction::vertical);
        });

        auto flexible_describer = [](layout_style::layout_flex_direction direction) {
            describe(layout_style::layout_flex_direction_enumeration.keys[(int)direction], [direction]() {

                auto create_layout = [](layout_style::layout_flex_direction direction, int x, int y, int w, int h, int childreans = 0, view_element c = {}) {
                    layout l;
                    l.style.direction = direction;
                    l.x = x;
                    l.y = y;
                    l.w = w;
                    l.h = h;
                    
                    for(int i = 0; i < childreans; i++) {
                        l.childreans.push_back(c);
                    }

                    return l;
                };

                it("should correctly position it's childreans", [=](){
                    layout l = create_layout(direction, 0, 0, 400, 400, 10, {0, 0, 10, 10});
                    l.calculate_layout(0, 0, 400, 400);

                    for(int i = 0; i < 10; i++) {
                        int flexible_attribute = direction == layout_style::layout_flex_direction::vertical ? l.childreans[i].y : l.childreans[i].x;
                        int fixed_attribute = direction == layout_style::layout_flex_direction::vertical ? l.childreans[i].x : l.childreans[i].y;

                        expect(flexible_attribute).to<eq>(10 * i);
                        expect(fixed_attribute).to<eq>(0);
                    }
                });
                it("should update childreans width/height based on direction", [direction](){
                    layout l;

                    l.style.direction = layout_style::layout_flex_direction::vertical;

                    for(int i = 0; i < 10; i++) {
                        l.childreans.push_back({0, 0, 0, 0});
                    }
                    
                    l.calculate_layout(0, 0, 400, 400);

                    for(int i = 0; i < 10; i++) {
                        int space = direction == layout_style::layout_flex_direction::vertical ? l.childreans[i].w : l.childreans[i].h;
                        expect(space).to<eq>(400);
                    }
                });
            });
        };

        flexible_describer(layout_style::layout_flex_direction::vertical);
        flexible_describer(layout_style::layout_flex_direction::horizontal);

        describe("old", [](){

            it("should not update childreans height", [](){
                layout l;

                l.style.direction = layout_style::layout_flex_direction::vertical;

                for(int i = 0; i < 10; i++) {
                    l.childreans.push_back({0, 0, 10, 10});
                }
                
                l.calculate_layout(0, 0, 400, 400);

                for(int i = 0; i < 10; i++) {
                    expect(l.childreans[i].h).to<eq>(10);
                }
            });

            it("should divide size equally on spacers elements", [](){
                layout l;

                l.style.direction = layout_style::layout_flex_direction::vertical;

                for(int i = 0; i < 3; i++) {
                    l.childreans.push_back({0, 0, 10, 10, { 1 }});
                }
                
                l.calculate_layout(0, 0, 400, 400);

                for(int i = 0; i < 3; i++) {
                    expect(l.childreans[i].h).to<eq>(400 / 3);
                }
            });

            it("should divide size accordily to the spacer elements", [](){
                layout l;

                l.style.direction = layout_style::layout_flex_direction::vertical;

                l.childreans.push_back({0, 0, 10, 10, { 1 }});
                l.childreans.push_back({0, 0, 10, 10, { 1 }});
                l.childreans.push_back({0, 0, 10, 10, { 2 }});
                
                l.calculate_layout(0, 0, 400, 400);

                expect(l.childreans[0].h).to<eq>(100);
                expect(l.childreans[1].h).to<eq>(100);
                expect(l.childreans[2].h).to<eq>(200);
            });
        });
    });
});