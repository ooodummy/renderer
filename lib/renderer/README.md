# Renderer

Basic 2D renderer with an implementation done using D3D11. Currently, there is a lot planned and a ton of improvements
to be done. Eventually speed will become a major concern but currently the challenge is adding all the functionality I
desire first.

Carbon will later seperated into a different repository I'm just being lazy rn.

### Features

- Drawing
  - Point
  - Line
  - Rectangle
    - Outline
    - Filled
    - Rounding
  - Arc
    - Outline
    - Filled
  - Circle
    - Outline
    - Filled
  - Polyline
    - Multiple cap and joint types to select from
  - Bezier curve
  - Textured rectangle
  - Scissoring
  - Color keying
- Types
  - Color HSV and RGBA
- Buffer swapping

### Previews

#### Bezier Curves

![](assets/bezier.png)

#### Polylines

![](assets/polyline.png)

### TODO

Here's a huge list for things I plan to get done, if I was motivated hopefully these would be able to get done soon but
that is not the case on hand here. Lots of the ideas bellow are bad but being questions just encase it's not one. If
you see anything that's outright terrible, maybe then say something since that would be helpful. I find that lots of my
questions and concerns are for performance when I'm probably wasting my time worrying about that at this point.

I don't think I will ever document either since I really don't care and if someone else is using it everything should
already be pretty simple to understand, and they could just take a peek on over at test/src.cpp.

Lots of the following was written at absurd hours of the night when my mental stability is decreasing and my code
quality drastically gets even worse.

#### Renderer

- D3D11 Implementation
  - Textures
    - Research general information about textures to get a better understanding of how things such as an SRV are
      handled and how I can decorate a texture after setting it in my pixel shader.
  - [ ] Make sure that proper formats are utilized in all the parts of the pipeline, I don't know how I should easily
    assess what works best for each situation though.
  - [ ] Resizing the depth view stencil causes an exception when the window is minimized.
  - [ ] Multisample anti-aliasing with the rasterizer settings.
    - What other ways can AA be accomplished, and how do the pros and cons of each compare?
  - [ ] Appropriate settings for the compatible hardware provided
    - Checking hardware rendering support
    - Appropriate display adapter
    - Maximum rasterizer quality
    - What else?
- Buffer class
  - Primitive geometry
    - Rectangles
      - Round ones that are filled should be changed a little so the triangle fans for its round edges start from
        the center, I think switching to this legit just reduces the vertex count by 1 or two. But I think the mesh
        also just looking proper is a nice benefit.
    - [ ] Optimize allocations done by creating vertices that then get moves into the vertex buffer.
    - For handling adding vertices on the vertex buffer we could reduce and have minimal allocations by creating
      whole static primitives at once directly onto the vertex buffer and what would benefit the most would be geometry
      that currently uses a vector to store all of its vertices due to its size not being static and me being too lazy to
      just use a heap array because I would rather do something such as this approach. The only issue is how would we
      define batching and decide when to split, as well and when to add things such as a degenerate triangle. This whole
      approach might just be a fever dream.
    - [ ] Until a proper solution is done, add_vertices should at least be improved, and I should remove my usage of
      std::vector entirely
    - [ ] Until an optimization like right above describes is done we should at least reserve the proper size for each
    - vector or switch to doing heap arrays.
    - [ ] Another optimization for these allocations could be instead having create arc get a reference to the
      vertices that it will then add the arc onto, doing so just sounds more proper, remember after doing this to
      probably rename create_arc to something more accurate like add_arc_vertices
    - [ ] Make sure drawing is at the correct position and that things are drawing with the correct thickness.
      - Are there any issues when something has an odd thickness due to things like MSAA potentially making edges
        transparent due to them being drawn on half pixels?
    - [ ] Z-Buffer respect, currently legitimately nothing is done for ordering since I assumed that order was handled
      if a Z-Buffer wasn't used in a way so that the start of the vertices would be in the back with the end at the start,
      this is apparently not the case though and they at the same Z level.
      - Can I make it do what I assumed happened, because the only other way I can think of Z depth being handled is
        storing the current depth in the buffer then either increasing or decreasing it with each add_vertices call and
        iterating all the vertices and setting their Z depth, to do this I also need to add depth to my vertex input
        format.
    - Special effects
      - What's a better word to describe these capabilities inside a renderer? Drawing does not sound appropriate
        neither does special effects.
      - [ ] Make sure the most inward scissor position is used when scissors are nested, issues that we could
        experience otherwise could be a thing inside a scissor having a nested scissor that is bigger then the less
        nested scissor when everything while a scissor clip is pushed should only be within that push.
      - [ ] There should be a separate pixel shader for each effect since that is the standard although maybe not
        considering we would need a new shader for each combination that effects could be used in.
      - [ ] Color keying currently is not done since the format I'm using makes comparison inside the pixel shader
        slightly difficult.
      - [ ] Find a good blur implementation and steal it :smiling_imp:.
      - [ ] Get others opinion on what order arguments should be in given there are currently some inconsistencies.
        I imagine that the final order will be its bounds and extra needed information such as position, size, angle,
        and thickness. Proceeding after that information should be it's color or colors, I think making all the
        geometry. The final arguments should ever really care to set, but it's still an option for customization, the
        amount of segments used in anything with a curve perfectly falls in these criteria.
    - [ ] Multicolored primitives should be written for every type and then the non-multicolor one might just call the
      multicolor one using the repeat color, this is laziness. But there is absolutely zero way I'm writing each twice!
    - Triangle strip order is irrelevant because there is no reasoning I believe to use backface culling in 2D
      rendering. Should meshes still be made with order in mind? And is there preferred ways to decorate primitives that we
      are not doing? It legit does not matter at all I'm just interested in knowing is the preferred starting position and
      pattern to go in for things such as rectangles. Looking at documentation graphic engines should provide the pattern
      most people like.
- Fonts
  - [ ] Font texture atlas generation
    - Instead of cramping a bunch of rectangles in and checking for intersections and all that it's probably best to
      make it just keep getting taller and having its width be the maximum glyph width in either axis. This may waste a
      little of memory but should make essentially no difference other than not requiring some algorithm to layout.
    - Is drawing multiple textures in a singular decorate call possible when using a texture atlas thanks to UVs? And how
      does UV mapping work when using an atlas?
  - [ ] Font glyph bitmap/texture creation with minimal binary footprint potentially taking GDI code or using something
    like FW1. Freetype is nice, but it's size ups the binary from less than 100kb to 1mb.
    - Preferable if it also supports wide characters and ones that are colored or contain multiple colors.
- Polyline
  - [ ] Improve intersection checks so overlap does not appear when not expected.
  - [ ] Add the other joint types back since they got removed when switching from using triangle list to triangle
    strips.

#### Carbon

- Layout engine
  - Grid container
    - [ ] More functionality similar to CSS grid containers.
  - Flex container
    - [ ] Flex line container system since both container and line are just base containers so how can I move the flex
      container children into the flex line then split the flex lines children into another line in the containers
      children or should I just not create flex lines. And just make flex containers somewhat 2D... if wrapping is enabled
      and the container content area is exceeded
  - Flex line
    - [ ] Measure content minimum then constrain
    - Children should be able to escape if minimum isn't clamped yet and use extra thickness from margin, etc
    - [ ] Fix shrinking since it currently doesn't properly recompute after changing free space when something cannot
      flex anymore
    - [ ] Optimize compute since I doubt the current way is optimal
      - I have no idea how this should be approached since I don't think there are many if any super complex
        calculations the only improvement I can easily think of is the next bullet.
    - [ ] Caching by marking dirty for when recomputing is needed
      - We have to set up setters for everything otherwise how do we know when to mark an item dirty since it's marked
        as dirty if a variable for it's size changes, that kinda sucks.
      - An idea I have that may just be stupid it could I hash the memory of the flex items flex properties and them
        just save the last value that was when computed and then before computing to check if it matches. This sounds a
        bit mental though, its 1am.
    - [ ] Cross axis
      - How can we determine the size on the cross axis would it just be the min content and nothing else? If it's not
        the min cross content size then on the cross axis should we have the ability to constrain its size?
      - [ ] Once we set up the size we should also use to align content for aligning content on the cross axis. Then
        for the baseline do we simply just use the cross axis size relative to the computing container of the first child.
    - [ ] Calculate the baseline
      - Should we store each axis baseline?
      - How would we plan to set it up making having a widget_item for text would be straightforward, then should text
        shrink if constrained or is that issue irrelevant because we could just scissor our container and popout.
    - [ ] Add justify_content_stretch just currently was never added since we will probably need to store another
      variable for if a flex item, so we know if it is flexible from its settings then solid and maybe another for if
      constrained from computing, currently we just store if its currently flexible which can be set to false when
      constraints get applied.
    - [ ] Would having keyword constructors using enums be a crime, this would allow constricting things with phrases
      like flex_none, flex_auto, flex_min_content, flex_fill, etc. This sounds more sinful.
  - Flex item
    - [ ] Way without adding drawing or input stuff for later on without needing to define the virtual here since stuff
      widgets do should not be known by the layout engine, just shouldn't be that way imo
  - Potentially abstract flex_item into inheriting from an item component, since flex_items are the most basic one we
    currently have and grid containers have no usage in the flex properties.
  - [ ] Maybe setup having inheritable options for all the same flex properties as CSS then when adding children simply
    check what's being inherited and set it. Although I don't think C++ is meant for variables to be treated this way.
  - [ ] Should initializer list be used or should we be explicit with constructing, I personally like initializer list
    more since it's compact, but you can be my judge.
  - [ ] Should we return a ptr to self when calling useless set functions for stuff like self->set()->set()
    - Probably not since it's pretty weird plus just why?
  - [ ] Be more clear with the usages and layout of axes since it's a bit confusing at times trying to decide what to
    do, also setup some more conversion functions for it, may just need to make the axis of axes public tbh.
  - [ ] Review constructors and compare to CSS shorthand since the same format is being following, and it's pretty
    similar.
  - [ ] Is there a way to fet the sum of every axis in a vector, so we don't need our own function for returning x+y+z
    if not then I'll probably just slap mine somewhere better.
- Themes
  - [ ] Add all the style sheet options that will be required
    - Should we store a box model for each type of widget we want to be customized in that way uniquely and what then
      what should be generalized?
  - [ ] More style options, is there a better way to init!?
- Input system
  - Keyboard key information
    - [ ] Pressed
    - [ ] Down/held
      - Down and held are not the same since held means it was down the previous frame.
    - [ ] Releases
    - [ ] Last key
  - Mouse information
    - [ ] Position
    - [ ] Button information
    - [ ] Vertical and horizontal scroll
  - [ ] Input over utilities then maybe pushing the input area should be an opinion, and we could have another for
    checking if hovered in scissored input area. This would be useful if we showed tooltips when test is hovered and that
    text exceeds the groupbox this way the tooltip would not appear when hovering to the side. Another fix could be
    allowing things like labels to shrink and to then just use the bounds of the label text container for input checks.
  - No idea what design to go with for this part maybe io struct or just make the data global
- Widgets
  - Builder
    - [ ] Maybe ability to load from something like XML
    - [ ] Complete control
  - [ ] Snap grid
    - Basically everything should be able to be dragged around so that's going to be fun to program.
  - [ ] Animation timing system
  - [ ] How should widgets be able to have tooltips added to them if flex_items don't have children?
    - Maybe call things that can be added to widgets that are non container types something IDK the word
- How in god's name should we handle ordering everything and complex events like creating a temporary popup window. I
  think the thing with the ability to make the popup could just store the popup and set it to visible or invisible then
  when changing a window visibility have a flag for bringing to front on that action then hide when closed.
- Refactor
  - [ ] Function naming
  - [ ] Variable naming
  - [ ] See what can be refactored and how it would improve, my understanding of different things like an abstract
    factory and all that is very poor.
- Cross-platform support
  - [ ] Probably need some defines for things like OSX, UNIX, Windows
  - [ ] Are the functions I'm using like fmodf the right one to use, what ones should I avoid?
- [ ] Do anon namespace before C functions just so it can easily be noticed, how many people like this?
- Learn about more optimization techniques.
- [ ] Make sure the correct container types are always being used.
- What computing can be done asynchronously in our program
- [ ] Properly mark variables as volatile to avoid caching issues when multithreading.
- What should be global?
- [ ] Convert to own CMake then just add inside of libs and just edit there, so they are not mixed
- [ ] Check everything when not sleep-deprived or in insane pain (both rn)
- Should we avoid things using the CRT such as vector?
- [ ] Take a break from this and/or have someone review everything and get their feedback and then use it.

#### Actual TODO to be done in the next few days

Hopefully this list is actually feasible.

- Shapes
  - How should I set up shape vertices and indices then how do I apply a mesh matrix, so I can translate the vertices without needing to change the vertices
    - When using a matrix would each need to be in it's own draw call? And how is it even done with D3D11?
  - Should each shape just have gradient options added?
    - Probably because otherwise I would need to write each shape twice.
    - How could we allow the gradient direction to be in any rotation? Do we calculate the rotation each vertex is at from the center point then ease the color?
      - That just wouldnt work and gradients should have a customizable strength, origin, and ability to have multiple colors.
    - A shader could be used but then I don't think batching would be doable.
      - A effect to push color/gradient im the pixel shader may be nice though.
  - Should I modify the vertices/rebuild the mesh when simply changing the color?
    - I think this would have to be done since each vertex stores a color and otherwise I would have to set some override color.
  - When actually drawing shapes should I just make a function for draw_shape that just adds a shapes vertices to the vertex buffer? And then when drawing say a widget in carbon have it store each of it's shapes as a member and then when decorating the widget just use draw_shape on each?
    - This would mean I would create a shape for every shape even as simple as a rectangle inside of a widget which sounds kindof bad.
      - Would just using draw_rectangle for when doing shapes as simple as that be bad? I think it would make almost no change in the sense of performance and would just simply needing to create a shape member.
        - Wouldn't this shape system mean that we would always need to do a heap allocation for creating the vertices for a primitive before moving them into the vertex buffer because each shape just has a heap array for it's own vertices.
          - Maybe making a static_shape class could solve this, so we could use a stack allocation for it's vertex buffer then just have heap_shape for ones that allocate any amount of vertices. Then the base_shape would just have a virtual that returns a pointer to its vertices.
  - How should textures be handled?
    - What's the best texture helper class design?
      - Load texture from file option.
  - Once we have the ability to create shapes and do it properly, things such as creating a polyline for Bézier curves opposed to incorrectly setting edges won't be an issue because we can just recalculate the mesh only after a change has been made such as the control points, thickness, or polyline cap settings.
    - Ear clipping then also looks a lot more doable when only computing the mesh when changed but doing ear clipping is still super expensive.
    - This makes me very happy.
  - How much worse is it drawing things as simple as rectangles as a shape?
    - Probably not that bad since we could still be doing less.
  - Estimate saved CPU time by not constantly recalculating vertices.
    - Mesure each calculate mesh time then multiply that by the amount of avoided uses.
  - Should shapes be marked as modified then computed before swapping buffers so we don't recompute over and over in the odd event that could happenn.
    - Probably not since this should only happen when they are used incorrectly, I think.
    - If this is done should we asynchronouly split it up? Or is it still not expensive enough to benefit from that.
  - Feels super nasty to do this type of rendering for things that are not retained and are immediate like when drawing ESP.
- Font manager
  - Setup texture for each glyph.
    - I'll just use Freetype to construct textures since it's really nice as a library, its flaw is that it increases binary size a ton but whatever.
    - I should be able to setup RGB textures fine but for greyscale textures I would just want to use the texture format that best matches that case which would be just using the R channel. Then how should I properly override this to be white?
  - For each font create a texture atlas then copy each glyphs texture into the atlas and store a glyph map in each font with information about where each is in the atlas and it's basic information.
  - Proper way to draw textured rectangles, and potentially batch them with instancing.
  - What can we do to allow drawing the same font at any size other then drawing glyphs as vertices.
  - Batch static text, instance dynamic text
    - How should these be done?
- Carbon
  - Create a label widget which makes each glyph in a string into its own flex_item then for each word make it into a flex_container that holds all these glyphs or just have each word be a flex_item.
    - If each glyph was a flex item it would probably make having support for languages that draw their text backwards much more simplistic since we could just change the justify_content to be the end.
      - I don't see another approach for this.
- Renderer
  - Buffer priority and sub buffers.
  - Doesn't really matter but I feel creating arc vertices should be designed with a better layout.
    - Especially if we wants to do something such as customizing the center for trangle fans.
    - Is the current logic for computing arc points optimal?
  - Improve line drawing options with different options for percentages on a line or just divide and step by the line length and color/thickness options count.
    - Line cap options.
      - Should either end be able to use a different cap?
  - Polyline point color and weight customization.
    - Properly calculate polyline vertex count or switch back to vector
    - How should points be passed to polyline?
      - This problem is what makes bellow a pretty nice option.
    - Should polyline not be a class and instead a function?
      - Maybe once shapes are done it should instead be one of those. But then how would we setup the bezier shape if polylines are a shape?
    - Maybe it would be nice to reduce the function complexity.
  - Finish abstracting everything. Currently I'm in the progress of doing this and it's not going that well since I feel on my own due to my limited understanding of graphics it's unorganized.
    - In the CMake define what implementation to use for the window helper.
      - I have no idea how this should be properly done.
  - Simplify the D3D11 pipeline so it's code is more legiable and organized.
    - Fix depth crashes and do it all properly.
  - Automatically set the segment count depending on an arcs radius so each segment is X pixels appart opposed to being an arbitrary value.
    - Have this segment pixel spacing be a value that is set inside the renderer.
  - Rounding options so it can be an aspect or pixel value.
    - For example if a shapes rounding was 0.25f and it was an aspect it's rounding pixel size would be 25% of it's minimum axis width.
- Layout engine
  - Recomputing can be avoided when some things are set such as position.
    - It's possible but would be annoying to write.
    - Do we just add the change in position to the pos of every item in the containers box model?
      - We don't need to position this way if any children are dirty. Since everything can then change.
      - Actually not that hard.
    - Also look into when some attributes are set if we can determine if anything else is going to then need to recompute.
  - Fix minimum constraints
    - Will be a lot easier to debug with text rendering being used as debug information.
  - Fix shrinking
    - How do we determine shrink amounts? I don't think it's done in the same way as grow.
  - Add align_content
    - Will need to see how cross size is determined probably will just be the min content.
      - Do we just add the basis content to the min content and and then clamp it or does the basis content not change the minimum size?
- Extra
  - How should we store a delta time for animations so we don't fetch epoch time for every single one.
  - Split easing functions instead of having one for all with a enum for the type.
    - This would probably just outright be better to do.
  - Review whole project and look for anything bad or anything that is just inferior to another way of doing it.
    - This rambling and constant desire to improve is that. I just need someone smarter to tell me what's done poorly.
  - Application class that just holds the renderer and window and initializes those respectively?
    - Kinda pointless
    - How should failures be thrown?
      - Error callback that we bind and then just do MessageBox?
  - Descriptive comments for more of the project code.
    - Understanding the layout engine is not that easy of a task.
  - What can be improved further?
    - Why do I care this much about the renderer?
      - OCD?
  - How should drop shadows be drawn?
    - Yes, the fancy ones.
  - SBO/SOO should always be done when heap allocations may be needed.
    - Will need my own simple small vector or to make up my mind and choose one that already exist.
  - Z buffer because depth issues caused by batching vertices.
    - Order them in add vertices?
      - What's the starting depth and how should it increment.
  - Are colors inaccurate and is HSV conversion still correct?
    - Is the trigonometric way of easing colors faster?
      - Probably not due to the cost of sin/cos.
  - Should I actually use pragma include guards?
  - Find build issues when a different compiler is used
    - Why am I using MSVC instead of MinGW/clang.
      - How should I specify parallel jobs to run.
  - Find what's defining min/max.
  - CMYK and HSL color conversion
  - [](https://balance.reckon.com/package/getting-started)
  - PPI/DPI