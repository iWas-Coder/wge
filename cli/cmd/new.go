package cmd

import (
  "os"
  "fmt"
  "errors"
  "reflect"
  "io/ioutil"
  "gopkg.in/yaml.v2"
  "github.com/spf13/cobra"
  "github.com/go-git/go-git/v5"
  "github.com/manifoldco/promptui"
)

type Config struct {
  ProjectName string `yaml:"projectName"`
  GeometryType string `yaml:"geometryType"`
}

var (
  cfg Config
  cfgFile string
  newCmd = &cobra.Command{
    Use:   "new",
    Short: "Create a game powered by WGE",
    Long:  `Initialize a project with either an interactive form or via a YAML configuration file where all needed game properties are specified.`,
    Run: func(cmd *cobra.Command, args []string) {
      if cfgFile != "" {
        if len(args) != 0 {
          fmt.Println("[ERROR]: Passing arguments alongside a config file is not compatible")
          os.Exit(1)
        }
        parseCfgFile()
      } else {
        if len(args) > 1 {
          fmt.Println("[ERROR]: Invalid number of arguments")
          os.Exit(1)
        } else if len(args) == 1 {
          cfg.ProjectName = args[0]
        }
        form()
      }
      cloneTemplate(selectTemplate())
    },
  }
)

func init() {
  newCmd.Flags().StringVarP(&cfgFile, "config", "c", "", "Use a predefined YAML config file")
}

func cloneTemplate(url string) {
  _, err := git.PlainClone(cfg.ProjectName, false, &git.CloneOptions{
    URL:      "https://" + url,
    Progress: os.Stdout,
  })
  if err != nil {
    fmt.Printf("[ERROR]: Git clone failed (%v)\n", err)
    os.Exit(1)
  }
}

func selectTemplate() string {
  yamlData, err := yaml.Marshal(&cfg)
  if err != nil {
    fmt.Printf("[ERROR]: YAML marshaling failed (%v)\n", err)
    os.Exit(1)
  }
  tpl := "github.com/iWas-Coder/"
  fmt.Println(string(yamlData))
  if cfg.GeometryType == "2D" {
    tpl += "wge-2d-game-template"
  } else if cfg.GeometryType == "3D" {
    tpl += "wge-3d-game-template"
  }
  fmt.Printf("Selected template: %s\n", tpl)
  return tpl
}

func parseCfgFile() {
  _, err := os.Stat(cfgFile)
  if err != nil {
    fmt.Printf("[ERROR]: Config file does not exist (%v)\n", err)
    os.Exit(1)
  }
  data, err := ioutil.ReadFile(cfgFile)
  if err != nil {
    fmt.Printf("[ERROR]: Config file could not be read (%v)\n", err)
    os.Exit(1)
  }
  err = yaml.Unmarshal(data, &cfg)
  if err != nil {
    fmt.Printf("[ERROR]: Config file could not be deserialized (%v)\n", err)
    os.Exit(1)
  }
  v := reflect.ValueOf(cfg)
  for i := 0; i < v.NumField(); i++ {
    fieldValue := v.Field(i).Interface()
    if fieldValue == "" {
      fmt.Printf("[ERROR]: Field '%s' in config file is empty\n", v.Type().Field(i).Name)
      os.Exit(1)
    }
  }
}

func form() {
  form_projectName()
  form_geometryType()
}

func form_projectName() {
  if cfg.ProjectName != "" {
    return
  }
  validate := func(input string) error {
    if len(input) <= 1 {
      return errors.New("Project name must have more than 1 character")
    }
    return nil
  }
  prompt := promptui.Prompt{
    Label: "What is your project named?",
    Validate: validate,
  }
  result, err := prompt.Run()
  if err != nil {
    fmt.Printf("[ERROR]: Prompt failed %v\n", err)
    os.Exit(1)
  }
  cfg.ProjectName = result
}

func form_geometryType() {
  prompt := promptui.Select{
    Label: "What type of geometry do you want to use?",
    Items: []string{"2D", "3D"},
  }
  _, result, err := prompt.Run()
  if err != nil {
    fmt.Printf("[ERROR]: Prompt failed %v\n", err)
    os.Exit(1)
  }
  cfg.GeometryType = result
}
