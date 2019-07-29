[@bs.config {jsx: 3}];
[%bs.raw {|require("./MarkdownEditor.css")|}];

module DraftEditor = {
  type editorState;

  [@bs.module "./ReactDraftEditor"] [@react.component]
  external make: (~content: string, ~onChange: string => unit) => React.element =
    "default";
};

module TextArea = {
  open Webapi.Dom;

  external unsafeAsHtmlInputElement: Dom.element => Dom.htmlInputElement =
    "%identity";

  let setStyleHeight: (string, Dom.htmlInputElement) => unit = [%raw
    "(height, element) => { element.style.height = height; return }"
  ];

  let element = id =>
    document
    |> Document.getElementById(id)
    |> OptionUtils.unwrapUnsafely
    |> unsafeAsHtmlInputElement;

  let selectionStart = id => element(id) |> HtmlInputElement.selectionStart;

  let selectionEnd = id => element(id) |> HtmlInputElement.selectionEnd;
};

type action =
  | Bold
  | Italics
  | Code;

type defaultView =
  | Preview
  | Edit;

let str = React.string;

let updateDescription = (description, setDescription, updateDescriptionCB) => {
  setDescription(_ => description);
  updateDescriptionCB(description);
};

let handleClick =
    (id, description, setDescription, updateDescriptionCB, action, event) => {
  event |> ReactEvent.Mouse.preventDefault;

  let actionString =
    switch (action) {
    | Bold => "**"
    | Italics => "*"
    | Code => "`"
    };

  let start = TextArea.selectionStart(id);
  let finish = TextArea.selectionEnd(id);
  let sel = Js.String.substring(~from=start, ~to_=finish, description);

  let newText =
    if (start != finish) {
      Js.String.substring(~from=0, ~to_=start, description)
      ++ actionString
      ++ sel
      ++ actionString
      ++ Js.String.substring(
           ~from=finish,
           ~to_=description |> Js.String.length,
           description,
         );
    } else {
      description
      ++ (
        switch (action) {
        | Bold => " **strong text** "
        | Italics => " *emphasized text* "
        | Code => " `enter code here` "
        }
      );
    };
  updateDescription(newText, setDescription, updateDescriptionCB);
};

let buttonTitle = action =>
  switch (action) {
  | Bold => "Bold"
  | Italics => "Italics"
  | Code => "Code"
  };

let buttonIcon = action =>
  <span>
    {
      switch (action) {
      | Bold => <i className="far fa-bold" />
      | Italics => <i className="far fa-italic" />
      | Code => <i className="far fa-code" />
      }
    }
  </span>;

type previewButtonPosition =
  | PositionRight
  | PositionLeft;

let buttons =
    (
      id,
      description,
      setDescription,
      updateDescriptionCB,
      preview,
      setPreview,
      previewButtonPosition,
    ) => {
  let classes = "markdown-button-group__button hover:bg-primary-100 hover:text-primary-400 focus:outline-none focus:text-primary-600";

  let previewOrEditButton =
    (
      switch (description) {
      | "" => React.null
      | _someDescription =>
        <button
          key="preview-button"
          className=classes
          onClick=(
            event => {
              ReactEvent.Mouse.preventDefault(event);
              setPreview(_ => !preview);
            }
          )>
          <FaIcon classes={preview ? "fab fa-markdown" : "far fa-eye"} />
          <span className="ml-2">
            {(preview ? "Edit Markdown" : "Preview") |> str}
          </span>
        </button>
      }
    )
    |> Array.make(1);

  let styleButtons =
    [|Bold, Italics, Code|]
    |> Array.map(action =>
         <button
           className=classes
           disabled=preview
           key={action |> buttonTitle}
           title={action |> buttonTitle}
           onClick={
             handleClick(
               id,
               description,
               setDescription,
               updateDescriptionCB,
               action,
             )
           }>
           {action |> buttonIcon}
         </button>
       );

  (
    switch (previewButtonPosition) {
    | PositionLeft => Array.append(previewOrEditButton, styleButtons)
    | PositionRight => Array.append(styleButtons, previewOrEditButton)
    }
  )
  |> React.array;
};

[@react.component]
let make =
    (
      ~textareaId=?,
      ~placeholder=?,
      ~updateDescriptionCB,
      ~value,
      ~label=?,
      ~profile,
      ~maxLength=1000,
      ~defaultView,
    ) => {
  let (description, setDescription) = React.useState(() => value);
  let (preview, setPreview) =
    React.useState(() =>
      switch (defaultView) {
      | Preview => true
      | Edit => false
      }
    );
  let (id, _setId) =
    React.useState(() =>
      switch (textareaId) {
      | Some(id) => id
      | None =>
        "markdown-editor-"
        ++ (Js.Math.random_int(100000, 999999) |> string_of_int)
      }
    );
  let (label, previewButtonPosition) =
    switch (label) {
    | Some(label) => (
        <label
          className="inline-block tracking-wide text-gray-900 text-xs font-semibold"
          htmlFor=id>
          {label |> str}
        </label>,
        PositionLeft,
      )
    | None => (React.null, PositionRight)
    };

  <div>
    <div className="flex justify-between items-end bg-white pb-2">
      label
      <div className="flex markdown-button-group">
        {
          buttons(
            id,
            description,
            setDescription,
            updateDescriptionCB,
            preview,
            setPreview,
            previewButtonPosition,
          )
        }
      </div>
    </div>
    {
      preview ?
        <MarkdownBlock
          markdown=description
          className="pb-3 pt-2 leading-normal text-sm px-3 border border-transparent bg-gray-100 markdown-editor-preview"
          profile
        /> :
        <DraftEditor
          content=description
          onChange={
            content =>
              updateDescription(content, setDescription, updateDescriptionCB)
          }
        />
    }
  </div>;
};

module Jsx2 = {
  let component = ReasonReact.statelessComponent("MarkDownEditor");

  let make =
      (
        ~placeholder,
        ~updateDescriptionCB,
        ~value,
        ~label,
        ~profile,
        ~maxLength,
        ~defaultView,
        children,
      ) =>
    ReasonReactCompat.wrapReactForReasonReact(
      make,
      makeProps(
        ~placeholder,
        ~updateDescriptionCB,
        ~value,
        ~label,
        ~profile,
        ~maxLength,
        ~defaultView,
        (),
      ),
      children,
    );
};