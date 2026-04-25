export module caerwyn.gui.layout.row;

export import caerwyn.gui.widget;
export import caerwyn.gui.layout.box;

export namespace caerwyn::gui
{

    class RowLayout final : public BoxLayout
    {
    public:
        RowLayout() : BoxLayout(Axis::Horizontal)
        {
        }
    };

} // namespace caerwyn::gui
