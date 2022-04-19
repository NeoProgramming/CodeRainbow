using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;

namespace CodeRainbow
{
	[Export(typeof(IWpfTextViewCreationListener))]
	[ContentType("text")]
	[TextViewRole(PredefinedTextViewRoles.Document)]
	internal sealed class CRHighlighterFactory : IWpfTextViewCreationListener
	{

		[Export(typeof(AdornmentLayerDefinition))]
		[Name("cr_vsext")]
		[Order(After = PredefinedAdornmentLayers.Selection, Before = PredefinedAdornmentLayers.Text)]
		[TextViewRole(PredefinedTextViewRoles.Document)]
		public AdornmentLayerDefinition editorAdornmentLayer = null;


		public void TextViewCreated(IWpfTextView textView)
		{
            // create a coloring object every time
			new CRHighlighter(textView);
		}
	}
}
