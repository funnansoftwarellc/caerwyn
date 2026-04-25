export module caerwyn.gui.layout.column;

export import caerwyn.gui.widget;
export import caerwyn.gui.layout.box;

export namespace caerwyn::gui
{

    class ColumnLayout final : public BoxLayout
    {
    public:
        ColumnLayout() : BoxLayout(Axis::Vertical)
        {
        }
    };

} // namespace caerwyn::gui
